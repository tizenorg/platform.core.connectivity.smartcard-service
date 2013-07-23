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
/* standard library header */
#include <unistd.h>
#include <glib.h>
#include <gio/gio.h>
#include <vector>
#include <string>
#include <sys/socket.h>

/* SLP library header */
#include "security-server.h"

/* local header */
#include "smartcard-types.h"
#include "Debug.h"
#include "ByteArray.h"
#include "ServerResource.h"
#include "GDBusHelper.h"
#include "ServerGDBus.h"

using namespace std;

namespace smartcard_service_api
{
	GDBusDispatcher::GDBusDispatcher() : Synchronous()
	{
	}

	GDBusDispatcher::~GDBusDispatcher()
	{
	}

	GDBusDispatcher &GDBusDispatcher::getInstance()
	{
		static GDBusDispatcher dispatcher;

		return dispatcher;
	}

	void GDBusDispatcher::_push(dispatcher_cb_t cb,
		const vector<void *> &params)
	{
		syncLock();

		q.push(make_pair(cb, params));
		_INFO("request pushed, count [%d]", q.size());

		if (q.size() == 1) {
			/* start dispatch */
			_INFO("start dispatcher");
			g_idle_add(&GDBusDispatcher::dispatch, this);
		}

		syncUnlock();
	}

	void GDBusDispatcher::push(dispatcher_cb_t cb,
		const vector<void *> &params)
	{
		GDBusDispatcher::getInstance()._push(cb, params);
	}

	gboolean GDBusDispatcher::dispatch(gpointer user_data)
	{
		GDBusDispatcher *dispatcher = (GDBusDispatcher *)user_data;
		gboolean result = false;

		_BEGIN();

		dispatcher->syncLock();

		pair<dispatcher_cb_t, vector<void *> > &job =
			dispatcher->q.front();

		dispatcher->syncUnlock();

		job.first(job.second);

		dispatcher->syncLock();

		dispatcher->q.pop();
		if (dispatcher->q.size() > 0) {
			_INFO("remaining messages : %d", dispatcher->q.size());

			result = true;
		} else {
			_INFO("dispatch finished");
		}

		dispatcher->syncUnlock();

		_END();

		return result;
	}

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

	static void name_owner_changed(GDBusProxy *proxy,
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

	static void _on_name_owner_changed(GDBusConnection *connection,
		const gchar *sender_name, const gchar *object_path,
		const gchar *interface_name, const gchar *signal_name,
		GVariant *parameters, gpointer user_data)
	{
		GVariantIter *iter;
		gchar *name;
		gchar *old_owner;
		gchar *new_owner;

		iter = g_variant_iter_new(parameters);

		g_variant_iter_next(iter, "s", &name);
		g_variant_iter_next(iter, "s", &old_owner);
		g_variant_iter_next(iter, "s", &new_owner);

		name_owner_changed((GDBusProxy *)connection,
			name, old_owner, new_owner, user_data);
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

		/* subscribe signal */
		g_dbus_connection_signal_subscribe(connection,
			"org.freedesktop.DBus", /* bus name */
			"org.freedesktop.DBus", /* interface */
			"NameOwnerChanged", /* member */
			"/org/freedesktop/DBus", /* path */
			NULL, /* arg0 */
			G_DBUS_SIGNAL_FLAGS_NONE,
			_on_name_owner_changed,
			NULL, NULL);

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

		connection = g_bus_get_sync(G_BUS_TYPE_SYSTEM, NULL, &error);
		if (connection != NULL) {
		} else {
			_ERR("Can not get connection %s", error->message);
			g_error_free(error);

			return false;
		}

		_init();

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

		_deinit();

		if (connection != NULL) {
			g_object_unref(connection);
			connection = NULL;
		}
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

	static bool _is_authorized_request(GVariant *privilege,
		const char *rights)
	{
		bool result = true;
#ifdef USER_SPACE_SMACK
		ByteArray temp;

		/* apply user space smack */
		GDBusHelper::convertVariantToByteArray(privilege, temp);

		result = (security_server_check_privilege_by_cookie(
			(char *)temp.getBuffer(),
			"smartcard-service",
			rights) == SECURITY_SERVER_API_SUCCESS);
#endif
		return result;
	}

	/* SEService *
	 *
	 *
	 */
	static GVariant *_reader_to_variant(
		vector<pair<unsigned int, string> > &readers)
	{
		GVariantBuilder builder;
		uint32_t i;

		g_variant_builder_init(&builder, G_VARIANT_TYPE("a(us)"));

		for (i = 0; i < readers.size(); i++) {
			g_variant_builder_add(&builder, "(us)",
				readers[i].first, readers[i].second.c_str());
		}

		return g_variant_builder_end(&builder);
	}

	static gboolean __process_se_service(SmartcardServiceSeService *object,
		GDBusMethodInvocation *invocation,
		void *user_data)
	{
		_INFO("[MSG_REQUEST_READERS]");

		gint result = SCARD_ERROR_OK;
		GVariant *readers = NULL;
		vector<pair<unsigned int, string> > list;
		unsigned int handle = IntegerHandle::INVALID_HANDLE;
		const char *name;

		ServerResource &resource = ServerResource::getInstance();

		name = g_dbus_method_invocation_get_sender(invocation);

		pid_t pid;

		/* load secure elements */
		resource.loadSecureElements();

		pid = ServerGDBus::getInstance().getPID(name);

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

					if (list.size() == 0) {
						_INFO("no secure elements");
					}
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

		/* response to client */
		smartcard_service_se_service_complete_se_service(object,
			invocation, result, handle, readers);

		return true;
	}

	static void _process_se_service(vector<void *> &params)
	{
		SmartcardServiceSeService *object;
		GDBusMethodInvocation *invocation;
		void *user_data;

		if (params.size() != 3) {
			_ERR("invalid parameter");

			return;
		}

		object = (SmartcardServiceSeService *)params[0];
		invocation = (GDBusMethodInvocation *)params[1];
		user_data = params[2];

		__process_se_service(object, invocation, user_data);

		g_object_unref(object);
		g_object_unref(invocation);
	}

	static gboolean _handle_se_service(SmartcardServiceSeService *object,
		GDBusMethodInvocation *invocation,
		GVariant *privilege,
		void *user_data)
	{
		vector<void *> params;

		/* apply user space smack */
		if (_is_authorized_request(privilege, "r") == true) {
			g_object_ref(object);
			params.push_back((void *)object);

			g_object_ref(invocation);
			params.push_back((void *)invocation);

			params.push_back((void *)user_data);

			GDBusDispatcher::push(_process_se_service, params);
		} else {
			vector<pair<unsigned int, string> > list;

			_ERR("access denied");

			/* response to client */
			smartcard_service_se_service_complete_se_service(object,
				invocation,
				SCARD_ERROR_SECURITY_NOT_ALLOWED,
				IntegerHandle::INVALID_HANDLE,
				_reader_to_variant(list));
		}

		return true;
	}

	static gboolean __process_shutdown(SmartcardServiceSeService *object,
		GDBusMethodInvocation *invocation,
		guint handle, void *user_data)
	{
		const char *name;

		_INFO("[MSG_REQUEST_SHUTDOWN]");

		name = g_dbus_method_invocation_get_sender(invocation);

		ServerResource &resource = ServerResource::getInstance();

		resource.removeService(name, handle);

		/* response to client */
		smartcard_service_se_service_complete_shutdown(object,
			invocation, SCARD_ERROR_OK);

		/* terminate */
		if (resource.getClientCount() == 0) {
			_INFO("no client connected. terminate server");

			g_main_loop_quit((GMainLoop *)resource.getMainLoopInstance());
		}

		return true;
	}

	static void _process_shutdown(vector<void *> &params)
	{
		SmartcardServiceSeService *object;
		GDBusMethodInvocation *invocation;
		guint handle;
		void *user_data;

		if (params.size() != 4) {
			_ERR("invalid parameter");

			return;
		}

		object = (SmartcardServiceSeService *)params[0];
		invocation = (GDBusMethodInvocation *)params[1];
		handle = (guint)params[2];
		user_data = params[3];

		__process_shutdown(object, invocation, handle, user_data);

		g_object_unref(object);
		g_object_unref(invocation);
	}

	static gboolean _handle_shutdown(SmartcardServiceSeService *object,
		GDBusMethodInvocation *invocation,
		GVariant *privilege,
		guint handle,
		void *user_data)
	{
		vector<void *> params;

		/* apply user space smack */
		if (_is_authorized_request(privilege, "r") == true) {
			g_object_ref(object);
			params.push_back((void *)object);

			g_object_ref(invocation);
			params.push_back((void *)invocation);

			params.push_back((void *)handle);
			params.push_back(user_data);

			GDBusDispatcher::push(_process_shutdown, params);
		} else {
			_ERR("access denied");

			/* response to client */
			smartcard_service_se_service_complete_shutdown(object,
				invocation, SCARD_ERROR_SECURITY_NOT_ALLOWED);
		}

		return true;
	}

	bool ServerGDBus::initSEService()
	{
		GError *error = NULL;

		seService = smartcard_service_se_service_skeleton_new();

		g_signal_connect(seService,
			"handle-se-service",
			G_CALLBACK(_handle_se_service),
			this);

		g_signal_connect(seService,
			"handle-shutdown",
			G_CALLBACK(_handle_shutdown),
			this);

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
	static gboolean __process_open_session(SmartcardServiceReader *object,
		GDBusMethodInvocation *invocation, guint service_id,
		guint reader_id, void *user_data)
	{
		unsigned int handle = IntegerHandle::INVALID_HANDLE;
		int result;
		const char *name;

		_INFO("[MSG_REQUEST_OPEN_SESSION]");

		ServerResource &resource = ServerResource::getInstance();

		name = g_dbus_method_invocation_get_sender(invocation);

		if (resource.isValidReaderHandle(reader_id)) {
			vector<ByteArray> temp;

			handle = resource.createSession(name,
				service_id,
				reader_id,
				temp,
				(void *)NULL);
			if (handle != IntegerHandle::INVALID_HANDLE) {
				result = SCARD_ERROR_OK;
			} else {
				_ERR("createSession failed [%d]", handle);

				result = SCARD_ERROR_OUT_OF_MEMORY;
			}
		} else {
			_ERR("request invalid reader handle [%d]", reader_id);

			result = SCARD_ERROR_ILLEGAL_PARAM;
		}

		/* response to client */
		smartcard_service_reader_complete_open_session(object,
			invocation, result, handle);

		return true;
	}

	static void _process_open_session(vector<void *> &params)
	{
		SmartcardServiceReader *object;
		GDBusMethodInvocation *invocation;
		guint service_id;
		guint reader_id;
		void *user_data;

		if (params.size() != 5) {
			_ERR("invalid parameter");

			return;
		}

		object = (SmartcardServiceReader *)params[0];
		invocation = (GDBusMethodInvocation *)params[1];
		service_id = (guint)params[2];
		reader_id = (guint)params[3];
		user_data = params[4];

		__process_open_session(object, invocation, service_id,
			reader_id, user_data);

		g_object_unref(object);
		g_object_unref(invocation);
	}

	static gboolean _handle_open_session(SmartcardServiceReader *object,
		GDBusMethodInvocation *invocation,
		GVariant *privilege,
		guint service_id,
		guint reader_id, void *user_data)
	{
		vector<void *> params;

		/* apply user space smack */
		if (_is_authorized_request(privilege, "r") == true) {
			g_object_ref(object);
			params.push_back((void *)object);

			g_object_ref(invocation);
			params.push_back((void *)invocation);

			params.push_back((void *)service_id);
			params.push_back((void *)reader_id);
			params.push_back(user_data);

			GDBusDispatcher::push(_process_open_session, params);
		} else {
			_ERR("access denied");

			/* response to client */
			smartcard_service_reader_complete_open_session(object,
				invocation,
				SCARD_ERROR_SECURITY_NOT_ALLOWED,
				IntegerHandle::INVALID_HANDLE);
		}

		return true;
	}

	bool ServerGDBus::initReader()
	{
		GError *error = NULL;

		reader = smartcard_service_reader_skeleton_new();

		g_signal_connect(reader,
			"handle-open-session",
			G_CALLBACK(_handle_open_session),
			this);

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
	static gboolean __process_close_session(SmartcardServiceSession *object,
		GDBusMethodInvocation *invocation, guint service_id,
		guint session_id, void *user_data)
	{
		const char *name;

		_INFO("[MSG_REQUEST_CLOSE_SESSION]");

		ServerResource &resource = ServerResource::getInstance();

		name = g_dbus_method_invocation_get_sender(invocation);

		if (resource.isValidSessionHandle(name, service_id,
			session_id)) {
			resource.removeSession(name, service_id,
				session_id);
		} else {
			_ERR("invalid parameters");
		}

		/* response to client */
		smartcard_service_session_complete_close_session(object,
			invocation, SCARD_ERROR_OK);

		return true;
	}

	static void _process_close_session(vector<void *> &params)
	{
		SmartcardServiceSession *object;
		GDBusMethodInvocation *invocation;
		guint service_id;
		guint session_id;
		void *user_data;

		if (params.size() != 5) {
			_ERR("invalid parameter");

			return;
		}

		object = (SmartcardServiceSession *)params[0];
		invocation = (GDBusMethodInvocation *)params[1];
		service_id = (guint)params[2];
		session_id = (guint)params[3];
		user_data = params[4];

		__process_close_session(object, invocation, service_id,
			session_id, user_data);

		g_object_unref(object);
		g_object_unref(invocation);
	}

	static gboolean _handle_close_session(SmartcardServiceSession *object,
		GDBusMethodInvocation *invocation,
		GVariant *privilege,
		guint service_id,
		guint session_id, void *user_data)
	{
		vector<void *> params;

		/* apply user space smack */
		if (_is_authorized_request(privilege, "r") == true) {
			g_object_ref(object);
			params.push_back((void *)object);

			g_object_ref(invocation);
			params.push_back((void *)invocation);

			params.push_back((void *)service_id);
			params.push_back((void *)session_id);
			params.push_back(user_data);

			GDBusDispatcher::push(_process_close_session, params);
		} else {
			_ERR("access denied");

			/* response to client */
			smartcard_service_session_complete_close_session(object,
				invocation, SCARD_ERROR_SECURITY_NOT_ALLOWED);
		}

		return true;
	}

	static gboolean __process_get_atr(SmartcardServiceSession *object,
		GDBusMethodInvocation *invocation, guint service_id,
		guint session_id, void *user_data)
	{
		int result;
		ByteArray resp;
		GVariant *atr = NULL;
		const char *name;

		_INFO("[MSG_REQUEST_GET_ATR]");

		ServerResource &resource = ServerResource::getInstance();

		name = g_dbus_method_invocation_get_sender(invocation);

		ServiceInstance *client = NULL;

		client = resource.getService(name, service_id);
		if (client != NULL) {
			Terminal *terminal;

			terminal = client->getTerminal(session_id);
			if (terminal != NULL) {
				int rv;

				if ((rv = terminal->getATRSync(resp)) == 0) {
					result = SCARD_ERROR_OK;
				} else {
					_ERR("getATRSync failed : name [%s], service_id [%d], session_id [%d]", name, service_id, session_id);

					result = rv;
				}
			} else {
				_ERR("getTerminal failed : name [%s], service_id [%d], session_id [%d]", name, service_id, session_id);

				result = SCARD_ERROR_UNAVAILABLE;
			}
		} else {
			_ERR("getClient failed : name [%s], service_id [%d], session_id [%d]", name, service_id, session_id);

			result = SCARD_ERROR_UNAVAILABLE;
		}

		atr = GDBusHelper::convertByteArrayToVariant(resp);

		/* response to client */
		smartcard_service_session_complete_get_atr(object, invocation,
			result, atr);

		return true;
	}

	static void _process_get_atr(vector<void *> &params)
	{
		SmartcardServiceSession *object;
		GDBusMethodInvocation *invocation;
		guint service_id;
		guint session_id;
		void *user_data;

		if (params.size() != 5) {
			_ERR("invalid parameter");

			return;
		}

		object = (SmartcardServiceSession *)params[0];
		invocation = (GDBusMethodInvocation *)params[1];
		service_id = (guint)params[2];
		session_id = (guint)params[3];
		user_data = params[4];

		__process_get_atr(object, invocation, service_id,
			session_id, user_data);

		g_object_unref(object);
		g_object_unref(invocation);
	}

	static gboolean _handle_get_atr(SmartcardServiceSession *object,
		GDBusMethodInvocation *invocation,
		GVariant *privilege,
		guint service_id,
		guint session_id, void *user_data)
	{
		vector<void *> params;

		/* apply user space smack */
		if (_is_authorized_request(privilege, "r") == true) {
			g_object_ref(object);
			params.push_back((void *)object);

			g_object_ref(invocation);
			params.push_back((void *)invocation);

			params.push_back((void *)service_id);
			params.push_back((void *)session_id);
			params.push_back(user_data);

			GDBusDispatcher::push(_process_get_atr, params);
		} else {
			ByteArray resp;

			_ERR("access denied");

			/* response to client */
			smartcard_service_session_complete_get_atr(
				object,
				invocation,
				SCARD_ERROR_SECURITY_NOT_ALLOWED,
				GDBusHelper::convertByteArrayToVariant(resp));
		}

		return true;
	}

	static gboolean __process_open_channel(SmartcardServiceSession *object,
		GDBusMethodInvocation *invocation, guint service_id,
		guint session_id, guint type, GVariant *aid, void *user_data)
	{
		int result = SCARD_ERROR_UNKNOWN;
		ByteArray resp;
		GVariant *response = NULL;
		unsigned int channelID = IntegerHandle::INVALID_HANDLE;
		const char *name;

		_INFO("[MSG_REQUEST_OPEN_CHANNEL]");

		ServerResource &resource = ServerResource::getInstance();

		name = g_dbus_method_invocation_get_sender(invocation);

		try
		{
			ByteArray tempAid;

			GDBusHelper::convertVariantToByteArray(aid,
				tempAid);

			channelID = resource.createChannel(name,
				service_id, session_id, type, tempAid);
			if (channelID != IntegerHandle::INVALID_HANDLE) {
				ServerChannel *temp;

				temp = (ServerChannel *)resource.getChannel(
					name, service_id, channelID);
				if (temp != NULL) {
					resp = temp->getSelectResponse();

					result = SCARD_ERROR_OK;
				}
			} else {
				_ERR("channel is null.");

				/* set error value */
				result = SCARD_ERROR_UNAVAILABLE;
			}
		}
		catch (ExceptionBase &e)
		{
			result = e.getErrorCode();
		}

		response = GDBusHelper::convertByteArrayToVariant(resp);

		/* response to client */
		smartcard_service_session_complete_open_channel(object,
			invocation, result, channelID, response);

		return true;
	}

	static void _process_open_channel(vector<void *> &params)
	{
		SmartcardServiceSession *object;
		GDBusMethodInvocation *invocation;
		guint service_id;
		guint session_id;
		guint type;
		GVariant *aid;
		void *user_data;

		if (params.size() != 7) {
			_ERR("invalid parameter");

			return;
		}

		object = (SmartcardServiceSession *)params[0];
		invocation = (GDBusMethodInvocation *)params[1];
		service_id = (guint)params[2];
		session_id = (guint)params[3];
		type = (guint)params[4];
		aid = (GVariant *)params[5];
		user_data = params[6];

		__process_open_channel(object, invocation, service_id,
			session_id, type, aid, user_data);

		g_object_unref(object);
		g_object_unref(invocation);
		g_object_unref(aid);
	}

	static gboolean _handle_open_channel(SmartcardServiceSession *object,
		GDBusMethodInvocation *invocation,
		GVariant *privilege,
		guint service_id,
		guint session_id, guint type, GVariant *aid, void *user_data)
	{
		vector<void *> params;

		/* apply user space smack */
		if (_is_authorized_request(privilege, "rw") == true) {
			g_object_ref(object);
			params.push_back((void *)object);

			g_object_ref(invocation);
			params.push_back((void *)invocation);

			params.push_back((void *)service_id);
			params.push_back((void *)session_id);
			params.push_back((void *)type);

			g_object_ref(aid);
			params.push_back((void *)aid);
			params.push_back(user_data);

			GDBusDispatcher::push(_process_open_channel, params);
		} else {
			ByteArray resp;

			_ERR("access denied");

			/* response to client */
			smartcard_service_session_complete_open_channel(object,
				invocation,
				SCARD_ERROR_SECURITY_NOT_ALLOWED,
				IntegerHandle::INVALID_HANDLE,
				GDBusHelper::convertByteArrayToVariant(resp));
		}

		return true;
	}

	bool ServerGDBus::initSession()
	{
		GError *error = NULL;

		session = smartcard_service_session_skeleton_new();

		g_signal_connect(session,
			"handle-close-session",
			G_CALLBACK(_handle_close_session),
			this);

		g_signal_connect(session,
			"handle-get-atr",
			G_CALLBACK(_handle_get_atr),
			this);

		g_signal_connect(session,
			"handle-open-channel",
			G_CALLBACK(_handle_open_channel),
			this);

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
	static gboolean __process_close_channel(SmartcardServiceChannel *object,
		GDBusMethodInvocation *invocation, guint service_id,
		guint channel_id, void *user_data)
	{
		int result;
		const char *name;

		_INFO("[MSG_REQUEST_CLOSE_CHANNEL]");

		ServerResource &resource = ServerResource::getInstance();

		name = g_dbus_method_invocation_get_sender(invocation);

		resource.removeChannel(name, service_id, channel_id);

		result = SCARD_ERROR_OK;

		/* response to client */
		smartcard_service_channel_complete_close_channel(object,
			invocation, result);

		return true;
	}

	static void _process_close_channel(vector<void *> &params)
	{
		SmartcardServiceChannel *object;
		GDBusMethodInvocation *invocation;
		guint service_id;
		guint channel_id;
		void *user_data;

		if (params.size() != 5) {
			_ERR("invalid parameter");

			return;
		}

		object = (SmartcardServiceChannel *)params[0];
		invocation = (GDBusMethodInvocation *)params[1];
		service_id = (guint)params[2];
		channel_id = (guint)params[3];
		user_data = params[4];

		__process_close_channel(object, invocation, service_id,
			channel_id, user_data);

		g_object_unref(object);
		g_object_unref(invocation);
	}

	static gboolean _handle_close_channel(SmartcardServiceChannel *object,
		GDBusMethodInvocation *invocation,
		GVariant *privilege,
		guint service_id, guint channel_id, void *user_data)
	{
		vector<void *> params;

		/* apply user space smack */
		if (_is_authorized_request(privilege, "r") == true) {
			g_object_ref(object);
			params.push_back((void *)object);

			g_object_ref(invocation);
			params.push_back((void *)invocation);

			params.push_back((void *)service_id);
			params.push_back((void *)channel_id);
			params.push_back(user_data);

			GDBusDispatcher::push(_process_close_channel, params);
		} else {
			_ERR("access denied");

			/* response to client */
			smartcard_service_channel_complete_close_channel(
				object,
				invocation,
				SCARD_ERROR_SECURITY_NOT_ALLOWED);
		}

		return true;
	}

	static gboolean __process_transmit(SmartcardServiceChannel *object,
		GDBusMethodInvocation *invocation,
		guint service_id,
		guint channel_id,
		GVariant *command,
		void *user_data)
	{
		int result;
		Channel *channel = NULL;
		ByteArray resp;
		GVariant *response = NULL;
		const char *name;

		_INFO("[MSG_REQUEST_TRANSMIT]");

		ServerResource &resource = ServerResource::getInstance();

		name = g_dbus_method_invocation_get_sender(invocation);

		channel = resource.getChannel(name, service_id, channel_id);
		if (channel != NULL) {
			int rv;
			ByteArray cmd;

			GDBusHelper::convertVariantToByteArray(command, cmd);

			rv = channel->transmitSync(cmd, resp);
			if (rv == 0) {
				result = SCARD_ERROR_OK;
			} else {
				_ERR("transmit failed [%d]", rv);

				result = rv;
			}
		} else {
			_ERR("invalid handle : name [%s], service_id [%d], channel_id [%d]", name, service_id, channel_id);

			result = SCARD_ERROR_UNAVAILABLE;
		}

		response = GDBusHelper::convertByteArrayToVariant(resp);

		/* response to client */
		smartcard_service_channel_complete_transmit(object, invocation,
			result, response);

		return true;
	}

	static void _process_transmit(vector<void *> &params)
	{
		SmartcardServiceChannel *object;
		GDBusMethodInvocation *invocation;
		guint service_id;
		guint channel_id;
		GVariant *command;
		void *user_data;

		if (params.size() != 6) {
			_ERR("invalid parameter");

			return;
		}

		object = (SmartcardServiceChannel *)params[0];
		invocation = (GDBusMethodInvocation *)params[1];
		service_id = (guint)params[2];
		channel_id = (guint)params[3];
		command = (GVariant *)params[4];
		user_data = params[5];

		__process_transmit(object, invocation, service_id,
			channel_id, command, user_data);

		g_object_unref(object);
		g_object_unref(invocation);
		g_object_unref(command);
	}

	static gboolean _handle_transmit(SmartcardServiceChannel *object,
		GDBusMethodInvocation *invocation,
		GVariant *privilege,
		guint service_id,
		guint channel_id,
		GVariant *command,
		void *user_data)
	{
		vector<void *> params;

		/* apply user space smack */
		if (_is_authorized_request(privilege, "r") == true) {
			/* enqueue message */
			g_object_ref(object);
			params.push_back((void *)object);

			g_object_ref(invocation);
			params.push_back((void *)invocation);

			params.push_back((void *)service_id);
			params.push_back((void *)channel_id);

			g_object_ref(command);
			params.push_back((void *)command);

			params.push_back(user_data);

			GDBusDispatcher::push(_process_transmit, params);
		} else {
			ByteArray resp;

			_ERR("access denied");

			/* response to client */
			smartcard_service_channel_complete_transmit(object,
				invocation,
				SCARD_ERROR_SECURITY_NOT_ALLOWED,
				GDBusHelper::convertByteArrayToVariant(resp));
		}

		return true;
	}

	bool ServerGDBus::initChannel()
	{
		GError *error = NULL;

		channel = smartcard_service_channel_skeleton_new();

		g_signal_connect(channel,
			"handle-close-channel",
			G_CALLBACK(_handle_close_channel),
			this);

		g_signal_connect(channel,
			"handle-transmit",
			G_CALLBACK(_handle_transmit),
			this);

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
