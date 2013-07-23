/*
 * Copyright (c) 2012, 2013 Samsung Electronics Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifdef USE_GDBUS
#include <unistd.h>
#include <glib.h>
#include <gio/gio.h>
#include <vector>
#include <string>
#include <sys/socket.h>

#include "smartcard-types.h"
#include "Debug.h"
#include "ByteArray.h"
#include "ServerResource.h"
#include "GDBusHelper.h"
#include "ServerGDBus.h"

using namespace std;

namespace smartcard_service_api
{
	ServerGDBus::ServerGDBus() : dbus_proxy(NULL), connection(NULL),
		seService(NULL), reader(NULL), session(NULL), channel(NULL)
	{
	}

	ServerGDBus::~ServerGDBus()
	{
		deinit();
	}

	ServerGDBus &ServerGDBus::getInstance()
	{
		static ServerGDBus serverGDBus;

		return serverGDBus;
	}

	void ServerGDBus::name_owner_changed(GDBusProxy *proxy,
		const gchar *name, const gchar *old_owner,
		const gchar *new_owner, void *user_data)
	{
		if (strlen(new_owner) == 0) {
			ClientInstance *client;

			ServerResource &resource = ServerResource::getInstance();

			client = resource.getClient(old_owner);
			if (client != NULL) {
				_INFO("terminated client, pid [%d]", client->getPID());
				resource.removeClient(old_owner);

				if (resource.getClientCount() == 0) {
					g_main_loop_quit((GMainLoop *)resource.getMainLoopInstance());
				}
			}
		}
	}

	bool ServerGDBus::_init()
	{
		GError *error = NULL;

		/* init default context */
		dbus_proxy = g_dbus_proxy_new_for_bus_sync(G_BUS_TYPE_SYSTEM,
			G_DBUS_PROXY_FLAGS_NONE,
			NULL, /* GDBusInterfaceInfo */
			"org.freedesktop.DBus",
			"/org/freedesktop/DBus",
			"org.freedesktop.DBus",
			NULL, /* GCancellable */
			&error);
		if (dbus_proxy == NULL)
		{
			_ERR("Can not create proxy : %s", error->message);
			g_error_free(error);

			return false;
		}

		g_signal_connect(dbus_proxy, "name-owner-changed",
				G_CALLBACK(&ServerGDBus::name_owner_changed),
				this);

		return true;
	}

	void ServerGDBus::_deinit()
	{
		if (dbus_proxy != NULL) {
			g_object_unref(dbus_proxy);
			dbus_proxy = NULL;
		}
	}

	bool ServerGDBus::init()
	{
		GError *error = NULL;

		_init();

		connection = g_bus_get_sync(G_BUS_TYPE_SYSTEM, NULL, &error);
		if (connection != NULL) {
		} else {
			_ERR("Can not get connection %s", error->message);
			g_error_free(error);

			return false;
		}

		initSEService();
		initReader();
		initSession();
		initChannel();

		return true;
	}

	void ServerGDBus::deinit()
	{
		deinitSEService();
		deinitReader();
		deinitSession();
		deinitChannel();

		if (connection != NULL) {
			g_object_unref(connection);
			connection = NULL;
		}

		_deinit();
	}

	static gboolean _call_get_connection_unix_process_id_sync(
		GDBusProxy *proxy, const gchar *arg_name, guint *out_pid,
		GCancellable *cancellable, GError **error) {
		GVariant *_ret;

		_ret = g_dbus_proxy_call_sync(proxy,
				"GetConnectionUnixProcessID",
				g_variant_new("(s)", arg_name),
				G_DBUS_CALL_FLAGS_NONE,
				-1, cancellable, error);
		if (_ret != NULL) {
			g_variant_get(_ret, "(u)", out_pid);
			g_variant_unref(_ret);
		}

		return _ret != NULL;
	}

	pid_t ServerGDBus::getPID(const char *name)
	{
		guint pid = 0;
		GError *error = NULL;

		if (_call_get_connection_unix_process_id_sync(
			(GDBusProxy *)dbus_proxy, name,
			&pid, NULL, &error) == true) {
		} else {
			_ERR("_g_freedesktop_dbus_call_get_connection_unix_process_id_sync failed  : %s", error->message);
			g_error_free(error);
		}

		return pid;
	}

	/* SEService *
	 *
	 *
	 */
	static GVariant *_reader_to_variant(vector<pair<unsigned int, string> > &readers)
	{
		GVariantBuilder builder;
		uint32_t i;

		g_variant_builder_init(&builder, G_VARIANT_TYPE("a(us)"));

		for (i = 0; i < readers.size(); i++) {
			g_variant_builder_add(&builder, "(us)", readers[i].first, readers[i].second.c_str());
		}

		return g_variant_builder_end(&builder);
	}

	static gboolean _handle_se_service(SmartcardServiceSeService *object,
		GDBusMethodInvocation *invocation)
	{
		_INFO("[MSG_REQUEST_READERS]");

		gint result = SCARD_ERROR_OK;
		GVariant *readers = NULL;
		vector<pair<unsigned int, string> > list;
		unsigned int handle = IntegerHandle::INVALID_HANDLE;

		ServerResource &resource = ServerResource::getInstance();

		/* load secure elements */
		resource.loadSecureElements();

		const char *name = g_dbus_method_invocation_get_sender(invocation);

		pid_t pid = ServerGDBus::getInstance().getPID(name);

		_INFO("service requested, pid [%d]", pid);

		if (pid > 0) {
			ClientInstance *instance;

			instance = resource.getClient(name);
			if (instance == NULL) {
				_INFO("create client instance, pid [%d]", pid);

				resource.createClient(name, pid);

				instance = resource.getClient(name);

				/* generate certification hashes */
				instance->generateCertificationHashes();
			}

			if (instance != NULL) {
				ServiceInstance *service;

				/* create service */
				service = resource.createService(name);
				if (service != NULL) {

					handle = service->getHandle();
					resource.getReaders(list);

				} else {
					_ERR("createService failed");

					result = SCARD_ERROR_OUT_OF_MEMORY;
				}
			} else {
				_ERR("client doesn't exist, pid [%d]", pid);

				result = SCARD_ERROR_OUT_OF_MEMORY;
			}
		} else {
			_ERR("invalid pid, [%d]", pid);

			result = SCARD_ERROR_IPC_FAILED;
		}

		readers = _reader_to_variant(list);
		if (list.size() > 0) {
		} else {
			_INFO("no secure elements");
		}

		/* response to client */
		smartcard_service_se_service_complete_se_service(object,
			invocation, result, handle, readers);

		return true;
	}

	static gboolean _handle_shutdown(SmartcardServiceSeService *object,
		GDBusMethodInvocation *invocation, guint handle)
	{
		_INFO("[MSG_REQUEST_SHUTDOWN]");

		const char *name = g_dbus_method_invocation_get_sender(invocation);

		ServerResource::getInstance().removeService(name, handle);

		/* response to client */
		smartcard_service_se_service_complete_shutdown(object,
			invocation, SCARD_ERROR_OK);

		return true;
	}

	bool ServerGDBus::initSEService()
	{
		GError *error = NULL;

		seService = smartcard_service_se_service_skeleton_new();

		g_signal_connect(seService,
				"handle-se-service",
				G_CALLBACK(_handle_se_service),
				NULL);

		g_signal_connect(seService,
				"handle-shutdown",
				G_CALLBACK(_handle_shutdown),
				NULL);

		if (g_dbus_interface_skeleton_export(
					G_DBUS_INTERFACE_SKELETON(seService),
					connection,
					"/org/tizen/SmartcardService/SeService",
					&error) == false)
		{
			_ERR("Can not skeleton_export %s", error->message);

			g_error_free(error);
			g_object_unref(seService);
			seService = NULL;

			return false;
		}

		return true;
	}

	void ServerGDBus::deinitSEService()
	{
		if (seService != NULL) {
			g_object_unref(seService);
			seService = NULL;
		}
	}

	void ServerGDBus::emitReaderInserted(unsigned int reader_id,
		const char *reader_name)
	{
		smartcard_service_se_service_emit_reader_inserted(
			SMARTCARD_SERVICE_SE_SERVICE(seService),
			reader_id, reader_name);
	}

	void ServerGDBus::emitReaderRemoved(unsigned int reader_id,
		const char *reader_name)
	{
		smartcard_service_se_service_emit_reader_removed(
			SMARTCARD_SERVICE_SE_SERVICE(seService),
			reader_id, reader_name);
	}

	/* Reader *
	 *
	 *
	 */
	static gboolean _handle_open_session(SmartcardServiceReader *object,
		GDBusMethodInvocation *invocation, guint service_id,
		guint reader_id)
	{
		unsigned int handle = IntegerHandle::INVALID_HANDLE;
		int result;

		_INFO("[MSG_REQUEST_OPEN_SESSION]");

		ServerResource &resource = ServerResource::getInstance();

		const char *name = g_dbus_method_invocation_get_sender(invocation);

		if (resource.isValidReaderHandle(reader_id))
		{
			vector<ByteArray> temp;

			handle = resource.createSession(name, service_id, reader_id, temp, (void *)NULL);
			if (handle != IntegerHandle::INVALID_HANDLE)
			{
				result = SCARD_ERROR_OK;
			}
			else
			{
				_ERR("createSession failed [%d]", handle);
				result = SCARD_ERROR_OUT_OF_MEMORY;
			}
		}
		else
		{
			_ERR("request invalid reader handle [%d]", reader_id);
			result = SCARD_ERROR_ILLEGAL_PARAM;
		}

		/* response to client */
		smartcard_service_reader_complete_open_session(object,
			invocation, result, handle);

		return true;
	}

	bool ServerGDBus::initReader()
	{
		GError *error = NULL;

		reader = smartcard_service_reader_skeleton_new();

		g_signal_connect(reader,
				"handle-open-session",
				G_CALLBACK(_handle_open_session),
				NULL);

		if (g_dbus_interface_skeleton_export(
					G_DBUS_INTERFACE_SKELETON(reader),
					connection,
					"/org/tizen/SmartcardService/Reader",
					&error) == false)
		{
			_ERR("Can not skeleton_export %s", error->message);

			g_error_free(error);
			g_object_unref(reader);
			reader = NULL;

			return false;
		}

		return true;
	}

	void ServerGDBus::deinitReader()
	{
		if (reader != NULL) {
			g_object_unref(reader);
			reader = NULL;
		}
	}

	/* Session *
	 *
	 *
	 */
	static gboolean _handle_close_session(SmartcardServiceSession *object,
		GDBusMethodInvocation *invocation, guint service_id,
		guint session_id)
	{
		_INFO("[MSG_REQUEST_CLOSE_SESSION]");

		ServerResource &resource = ServerResource::getInstance();

		const char *name = g_dbus_method_invocation_get_sender(invocation);

		if (resource.isValidSessionHandle(name, service_id, session_id))
		{
			resource.removeSession(name, service_id, session_id);
		}

		/* response to client */
		smartcard_service_session_complete_close_session(object,
			invocation, SCARD_ERROR_OK);

		return true;
	}

	static gboolean _handle_get_atr(SmartcardServiceSession *object,
		GDBusMethodInvocation *invocation, guint service_id,
		guint session_id)
	{
		int result;
		GVariant *atr = NULL;
		ServiceInstance *client = NULL;

		_INFO("[MSG_REQUEST_GET_ATR]");

		ServerResource &resource = ServerResource::getInstance();

		const char *name = g_dbus_method_invocation_get_sender(invocation);

		if ((client = resource.getService(name, service_id)) != NULL)
		{
			Terminal *terminal = NULL;

			if ((terminal = client->getTerminal(session_id)) != NULL)
			{
				int rv;
				ByteArray temp;

				if ((rv = terminal->getATRSync(temp)) == 0)
				{
					atr = GDBusHelper::convertByteArrayToVariant(temp);
					result = SCARD_ERROR_OK;
				}
				else
				{
					_ERR("transmit failed [%d]", rv);

					result = rv;
				}
			}
			else
			{
				_ERR("getTerminal failed : name [%s], service_id [%d], session_id [%d]", name, service_id, session_id);
				result = SCARD_ERROR_UNAVAILABLE;
			}
		}
		else
		{
			_ERR("getClient failed : name [%s], service_id [%d], session_id [%d]", name, service_id, session_id);
			result = SCARD_ERROR_UNAVAILABLE;
		}

		if (atr == NULL) {
			atr = GDBusHelper::convertByteArrayToVariant(ByteArray::EMPTY);
		}

		/* response to client */
		smartcard_service_session_complete_get_atr(object, invocation,
			result, atr);

		return true;
	}

	static gboolean _handle_open_channel(SmartcardServiceSession *object,
		GDBusMethodInvocation *invocation, guint service_id,
		guint session_id, guint type, GVariant *aid)
	{
		int result = SCARD_ERROR_UNKNOWN;
		GVariant *response = NULL;
		unsigned int channelID = IntegerHandle::INVALID_HANDLE;

		_INFO("[MSG_REQUEST_OPEN_CHANNEL]");

		ServerResource &resource = ServerResource::getInstance();

		const char *name = g_dbus_method_invocation_get_sender(invocation);

		try
		{
			ByteArray tempAid;

			GDBusHelper::convertVariantToByteArray(aid, tempAid);

			channelID = resource.createChannel(name, service_id, session_id, type, tempAid);
			if (channelID != IntegerHandle::INVALID_HANDLE)
			{
				ServerChannel *temp;

				temp = (ServerChannel *)resource.getChannel(name, service_id, channelID);
				if (temp != NULL)
				{
					result = SCARD_ERROR_OK;
					response = GDBusHelper::convertByteArrayToVariant(temp->getSelectResponse());
				}
			}
			else
			{
				_ERR("channel is null.");

				/* set error value */
				result = SCARD_ERROR_UNAVAILABLE;
			}
		}
		catch (ExceptionBase &e)
		{
			result = e.getErrorCode();
		}

		if (response == NULL) {
			response = GDBusHelper::convertByteArrayToVariant(ByteArray::EMPTY);
		}
		/* response to client */
		smartcard_service_session_complete_open_channel(object,
			invocation, result, channelID, response);

		return true;
	}

	bool ServerGDBus::initSession()
	{
		GError *error = NULL;

		session = smartcard_service_session_skeleton_new();

		g_signal_connect(session,
				"handle-close-session",
				G_CALLBACK(_handle_close_session),
				NULL);

		g_signal_connect(session,
				"handle-get-atr",
				G_CALLBACK(_handle_get_atr),
				NULL);

		g_signal_connect(session,
				"handle-open-channel",
				G_CALLBACK(_handle_open_channel),
				NULL);

		if (g_dbus_interface_skeleton_export(
					G_DBUS_INTERFACE_SKELETON(session),
					connection,
					"/org/tizen/SmartcardService/Session",
					&error) == false)
		{
			_ERR("Can not skeleton_export %s", error->message);

			g_error_free(error);
			g_object_unref(session);
			session = NULL;

			return false;
		}

		return true;
	}

	void ServerGDBus::deinitSession()
	{
		if (session != NULL) {
			g_object_unref(session);
			session = NULL;
		}
	}

	/* Channel *
	 *
	 *
	 */
	static gboolean _handle_close_channel(SmartcardServiceChannel *object,
		GDBusMethodInvocation *invocation, guint service_id,
		guint channel_id)
	{
		int result;

		_INFO("[MSG_REQUEST_CLOSE_CHANNEL]");

		ServerResource &resource = ServerResource::getInstance();

		const char *name = g_dbus_method_invocation_get_sender(invocation);

		result = SCARD_ERROR_OK;

		if (resource.getChannel(name, service_id, channel_id) != NULL)
		{
			resource.removeChannel(name, service_id, channel_id);
		}

		/* response to client */
		smartcard_service_channel_complete_close_channel(object,
			invocation, result);

		return true;
	}

	static gboolean _handle_transmit(SmartcardServiceChannel *object,
		GDBusMethodInvocation *invocation, guint service_id,
		guint channel_id, GVariant *command)
	{
		int result;
		Channel *channel = NULL;
		GVariant *response = NULL;

		_INFO("[MSG_REQUEST_TRANSMIT]");

		ServerResource &resource = ServerResource::getInstance();

		const char *name = g_dbus_method_invocation_get_sender(invocation);

		if ((channel = resource.getChannel(name, service_id, channel_id)) != NULL)
		{
			int rv;
			ByteArray cmd, resp;

			GDBusHelper::convertVariantToByteArray(command, cmd);

			if ((rv = channel->transmitSync(cmd, resp)) == 0)
			{
				response = GDBusHelper::convertByteArrayToVariant(resp);
				result = SCARD_ERROR_OK;
			}
			else
			{
				_ERR("transmit failed [%d]", rv);
				result = rv;
			}
		}
		else
		{
			_ERR("invalid handle : name [%s], service_id [%d], channel_id [%d]", name, service_id, channel_id);
			result = SCARD_ERROR_UNAVAILABLE;
		}

		if (response == NULL) {
			response = GDBusHelper::convertByteArrayToVariant(ByteArray::EMPTY);
		}

		/* response to client */
		smartcard_service_channel_complete_transmit(object, invocation,
			result, response);

		return true;
	}

	bool ServerGDBus::initChannel()
	{
		GError *error = NULL;

		channel = smartcard_service_channel_skeleton_new();

		g_signal_connect(channel,
				"handle-close-channel",
				G_CALLBACK(_handle_close_channel),
				NULL);

		g_signal_connect(channel,
				"handle-transmit",
				G_CALLBACK(_handle_transmit),
				NULL);

		if (g_dbus_interface_skeleton_export(
					G_DBUS_INTERFACE_SKELETON(channel),
					connection,
					"/org/tizen/SmartcardService/Channel",
					&error) == false)
		{
			_ERR("Can not skeleton_export %s", error->message);

			g_error_free(error);
			g_object_unref(channel);
			channel = NULL;

			return false;
		}

		return true;
	}

	void ServerGDBus::deinitChannel()
	{
		if (channel != NULL) {
			g_object_unref(channel);
			channel = NULL;
		}
	}
} /* namespace smartcard_service_api */
#endif
