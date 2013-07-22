/*
 * Copyright (c) 2012 Samsung Electronics Co., Ltd All Rights Reserved
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

/* standard library header */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

/* SLP library header */

/* local header */
#include "Debug.h"
#include "Session.h"
#include "Reader.h"
#include "ClientChannel.h"
#ifdef USE_GDBUS
#include "ClientGDBus.h"
#else
#include "ClientIPC.h"
#endif

#ifndef EXTERN_API
#define EXTERN_API __attribute__((visibility("default")))
#endif

namespace smartcard_service_api
{
	Session::Session(void *context, Reader *reader, void *handle) :
		SessionHelper(reader)
	{
		this->context = NULL;

		if (context == NULL || handle == NULL)
		{
			_ERR("handle is null");

			return;
		}

		this->context = context;
		this->handle = handle;

#ifdef USE_GDBUS
		/* initialize client */
		if (!g_thread_supported())
		{
			g_thread_init(NULL);
		}

		g_type_init();

		/* init default context */
		GError *error = NULL;

		proxy = smartcard_service_session_proxy_new_for_bus_sync(
			G_BUS_TYPE_SYSTEM, G_DBUS_PROXY_FLAGS_NONE,
			"org.tizen.SmartcardService",
			"/org/tizen/SmartcardService/Session",
			NULL, &error);
		if (proxy == NULL)
		{
			_ERR("Can not create proxy : %s", error->message);
			g_error_free(error);
			return;
		}
#endif
		closed = false;
	}

	Session::~Session()
	{
		size_t i;

		closeSync();

		for (i = 0; i < channels.size(); i++)
		{
			delete (ClientChannel *)channels[i];
		}

		channels.clear();
	}

	void Session::closeChannels() throw (ErrorIO &, ErrorIllegalState &)
	{
		size_t i;

		for (i = 0; i < channels.size(); i++)
		{
			channels[i]->closeSync();
		}
	}
#ifdef USE_GDBUS
	void Session::session_get_atr_cb(GObject *source_object,
		GAsyncResult *res, gpointer user_data)
	{
		CallbackParam *param = (CallbackParam *)user_data;
		Session *session;
		getATRCallback callback;
		gint result;
		GVariant *var_atr;
		GError *error = NULL;
		ByteArray atr;

		_INFO("MSG_REQUEST_GET_ATR");

		if (param == NULL) {
			_ERR("null parameter!!!");
			return;
		}

		session = (Session *)param->instance;
		callback = (getATRCallback)param->callback;

		if (smartcard_service_session_call_get_atr_finish(
			SMARTCARD_SERVICE_SESSION(source_object),
			&result, &var_atr, res, &error) == true) {
			if (result == SCARD_ERROR_OK) {
				GDBusHelper::convertVariantToByteArray(var_atr, atr);

				session->atr = atr;
			} else {
				_ERR("smartcard_service_session_call_get_atr failed, [%d]", result);
			}
		} else {
			_ERR("smartcard_service_session_call_get_atr failed, [%s]", error->message);
			g_error_free(error);

			result = SCARD_ERROR_IPC_FAILED;
		}

		if (callback != NULL) {
			callback(atr.getBuffer(),
				atr.size(), result, param->user_param);
		}

		delete param;
	}

	void Session::session_open_channel_cb(GObject *source_object,
		GAsyncResult *res, gpointer user_data)
	{
		CallbackParam *param = (CallbackParam *)user_data;
		Session *session;
		openChannelCallback callback;
		gint result;
		guint channel_id;
		GVariant *var_response;
		GError *error = NULL;
		Channel *channel;

		_INFO("MSG_REQUEST_OPEN_CHANNEL");

		if (param == NULL) {
			_ERR("null parameter!!!");
			return;
		}

		session = (Session *)param->instance;
		callback = (openChannelCallback)param->callback;

		if (smartcard_service_session_call_open_channel_finish(
			SMARTCARD_SERVICE_SESSION(source_object),
			&result, &channel_id, &var_response,
			res, &error) == true) {
			if (result == SCARD_ERROR_OK) {
				ByteArray response;

				GDBusHelper::convertVariantToByteArray(
					var_response, response);

				/* create new instance of channel */
				channel = new ClientChannel(session->context,
					session, channel_id,
					response, (void *)channel_id);
				if (channel != NULL) {
					session->channels.push_back(channel);
				} else {
					_ERR("alloc failed");

					result = SCARD_ERROR_OUT_OF_MEMORY;
				}
			} else {
				_ERR("smartcard_service_session_call_open_channel failed, [%d]", result);
			}
		} else {
			_ERR("smartcard_service_session_call_open_channel failed, [%s]", error->message);
			g_error_free(error);

			result = SCARD_ERROR_IPC_FAILED;
		}

		if (callback != NULL) {
			callback(channel, result, param->user_param);
		}

		delete param;
	}

	void Session::session_close_cb(GObject *source_object,
		GAsyncResult *res, gpointer user_data)
	{
		CallbackParam *param = (CallbackParam *)user_data;
		Session *session;
		closeSessionCallback callback;
		gint result;
		GError *error = NULL;

		_INFO("MSG_REQUEST_CLOSE_SESSION");

		if (param == NULL) {
			_ERR("null parameter!!!");
			return;
		}

		session = (Session *)param->instance;
		callback = (closeSessionCallback)param->callback;

		if (smartcard_service_session_call_close_session_finish(
			SMARTCARD_SERVICE_SESSION(source_object),
			&result, res, &error) == true) {
			if (result == SCARD_ERROR_OK) {
				session->closed = true;
			} else {
				_ERR("smartcard_service_session_call_close_session failed, [%d]", result);
			}
		} else {
			_ERR("smartcard_service_session_call_close_session failed, [%s]", error->message);
			g_error_free(error);

			result = SCARD_ERROR_IPC_FAILED;
		}

		if (callback != NULL) {
			callback(result, param->user_param);
		}

		delete param;
	}
#endif
	const ByteArray Session::getATRSync()
		throw (ExceptionBase &, ErrorIO &, ErrorSecurity &,
			ErrorIllegalState &, ErrorIllegalParameter &)
	{
		ByteArray result;

		if (getReader()->isSecureElementPresent() == true)
		{
			if (atr.isEmpty() == true)
			{
#ifdef USE_GDBUS
				gint ret;
				GVariant *var_atr = NULL;
				GError *error = NULL;

				if (smartcard_service_session_call_get_atr_sync(
					(SmartcardServiceSession *)proxy,
					GPOINTER_TO_UINT(context),
					GPOINTER_TO_UINT(handle),
					&ret, &var_atr, NULL, &error) == true) {
					if (ret == SCARD_ERROR_OK) {
						GDBusHelper::convertVariantToByteArray(var_atr, result);

						atr = result;
					} else {
						_ERR("smartcard_service_session_call_get_atr_sync failed, [%d]", ret);

						THROW_ERROR(ret);
					}
				} else {
					_ERR("smartcard_service_session_call_get_atr_sync failed, [%s]", error->message);
					g_error_free(error);

					THROW_ERROR(SCARD_ERROR_IPC_FAILED);
				}
#else
				Message msg;
				int rv;
#ifdef CLIENT_IPC_THREAD
				/* request channel handle from server */
				msg.message = Message::MSG_REQUEST_GET_ATR;
				msg.param1 = (unsigned long)handle;
				msg.error = (unsigned long)context; /* using error to context */
				msg.caller = (void *)this;
				msg.callback = (void *)this; /* if callback is class instance, it means synchronized call */

				syncLock();
				if (ClientIPC::getInstance().sendMessage(msg) == true)
				{
					rv = waitTimedCondition(0);
					if (rv != 0)
					{
						_ERR("time over");
						this->error = SCARD_ERROR_OPERATION_TIMEOUT;
					}
				}
				else
				{
					_ERR("sendMessage failed");
					this->error = SCARD_ERROR_IPC_FAILED;
				}
				syncUnlock();

				if (this->error != SCARD_ERROR_OK)
				{
					ThrowError::throwError(this->error);
				}
#endif
#endif
			}

			result = atr;
		}
		else
		{
			_ERR("unavailable session");
			throw ErrorIllegalState(SCARD_ERROR_UNAVAILABLE);
		}

		return result;
	}

	int Session::getATR(getATRCallback callback, void *userData)
	{
		int result;

		if (getReader()->isSecureElementPresent() == true)
		{
			if (atr.isEmpty() == true)
			{
#ifdef USE_GDBUS
				CallbackParam *param = new CallbackParam();

				param->instance = this;
				param->callback = (void *)callback;
				param->user_param = userData;

				smartcard_service_session_call_get_atr(
					(SmartcardServiceSession *)proxy,
					GPOINTER_TO_UINT(context),
					GPOINTER_TO_UINT(handle), NULL,
					&Session::session_get_atr_cb, param);

				result = SCARD_ERROR_OK;
#else
				Message msg;

				/* request channel handle from server */
				msg.message = Message::MSG_REQUEST_GET_ATR;
				msg.param1 = (unsigned long)handle;
				msg.error = (unsigned long)context; /* using error to context */
				msg.caller = (void *)this;
				msg.callback = (void *)callback;
				msg.userParam = userData;

				if (ClientIPC::getInstance().sendMessage(msg) == true)
				{
					result = SCARD_ERROR_OK;
				}
				else
				{
					_ERR("sendMessage failed");
					result = SCARD_ERROR_IPC_FAILED;
				}
#endif
			}
			else
			{
				result = SCARD_ERROR_OK;

				/* TODO : invoke callback directly */
				callback(atr.getBuffer(),
					atr.size(), 0, userData);
			}
		}
		else
		{
			_ERR("unavailable session");
			result = SCARD_ERROR_ILLEGAL_STATE;
		}

		return result;
	}

	void Session::closeSync()
		throw (ExceptionBase &, ErrorIO &, ErrorSecurity &,
			ErrorIllegalState &, ErrorIllegalParameter &)
	{
		if (isClosed() == false)
		{
			closed = true;
			closeChannels();
#ifdef USE_GDBUS
			gint ret;
			GError *error = NULL;

			if (smartcard_service_session_call_close_session_sync(
				(SmartcardServiceSession *)proxy,
				GPOINTER_TO_UINT(context),
				GPOINTER_TO_UINT(handle),
				&ret, NULL, &error) == true) {
				if (ret == SCARD_ERROR_OK) {
					closed = true;
				} else {
					_ERR("smartcard_service_session_call_close_session_sync failed, [%d]", ret);

					THROW_ERROR(ret);
				}
			} else {
				_ERR("smartcard_service_session_call_get_atr_sync failed, [%s]", error->message);
				g_error_free(error);

				THROW_ERROR(SCARD_ERROR_IPC_FAILED);
			}
#else
			int rv;
			Message msg;

#ifdef CLIENT_IPC_THREAD
			/* request channel handle from server */
			msg.message = Message::MSG_REQUEST_CLOSE_SESSION;
			msg.param1 = (unsigned long)handle;
			msg.error = (unsigned long)context; /* using error to context */
			msg.caller = (void *)this;
			msg.callback = (void *)this; /* if callback is class instance, it means synchronized call */

			syncLock();
			if (ClientIPC::getInstance().sendMessage(msg) == true)
			{
				rv = waitTimedCondition(0);

				if (rv != 0)
				{
					_ERR("time over");
					this->error = SCARD_ERROR_OPERATION_TIMEOUT;
				}
			}
			else
			{
				_ERR("sendMessage failed");
				this->error = SCARD_ERROR_IPC_FAILED;
			}
			syncUnlock();

			if (this->error != SCARD_ERROR_OK)
			{
				ThrowError::throwError(this->error);
			}
#endif
#endif
		}
	}

	int Session::close(closeSessionCallback callback, void *userData)
	{
		int result = SCARD_ERROR_OK;

		if (isClosed() == false)
		{
			closed = true;
			closeChannels();
#ifdef USE_GDBUS
			CallbackParam *param = new CallbackParam();

			param->instance = this;
			param->callback = (void *)callback;
			param->user_param = userData;

			smartcard_service_session_call_close_session(
				(SmartcardServiceSession *)proxy,
				GPOINTER_TO_UINT(context),
				GPOINTER_TO_UINT(handle), NULL,
				&Session::session_close_cb, param);
#else
			Message msg;
			/* request channel handle from server */
			msg.message = Message::MSG_REQUEST_CLOSE_SESSION;
			msg.param1 = (unsigned long)handle;
			msg.error = (unsigned long)context; /* using error to context */
			msg.caller = (void *)this;
			msg.callback = (void *)callback;
			msg.userParam = userData;

			if (ClientIPC::getInstance().sendMessage(msg) == false)
			{
				_ERR("sendMessage failed");
				result = SCARD_ERROR_IPC_FAILED;
			}
#endif
		}

		return result;
	}

	unsigned int Session::getChannelCountSync()
	{
		unsigned int count = 0;

		if (getReader()->isSecureElementPresent() == true)
		{
#ifdef USE_GDBUS
			count = channels.size();
#else
			Message msg;
			int rv;

			channelCount = -1;
#ifdef CLIENT_IPC_THREAD
			/* request channel handle from server */
			msg.message = Message::MSG_REQUEST_GET_CHANNEL_COUNT;
			msg.param1 = (unsigned long)handle;
			msg.error = (unsigned long)context; /* using error to context */
			msg.caller = (void *)this;
			msg.callback = (void *)this; /* if callback is class instance, it means synchronized call */

			syncLock();
			if (ClientIPC::getInstance().sendMessage(msg) == true)
			{
				rv = waitTimedCondition(0);
				if (rv != 0)
				{
					_ERR("time over");
					this->error = SCARD_ERROR_OPERATION_TIMEOUT;
				}

				count = channelCount;
			}
			else
			{
				_ERR("sendMessage failed");
				this->error = SCARD_ERROR_IPC_FAILED;
			}
			syncUnlock();

			if (this->error != SCARD_ERROR_OK)
			{
				ThrowError::throwError(this->error);
			}
#endif
#endif
		}
		else
		{
			_ERR("unavailable session");
			throw ErrorIllegalState(SCARD_ERROR_UNAVAILABLE);
		}

		return count;
	}

	int Session::getChannelCount(getChannelCountCallback callback, void *userData)
	{
		int result;

		if (getReader()->isSecureElementPresent() == true)
		{
#ifdef USE_GDBUS
#else
			Message msg;

			msg.message = Message::MSG_REQUEST_GET_CHANNEL_COUNT;
			msg.param1 = (unsigned long)handle;
			msg.error = (unsigned long)context; /* using error to context */
			msg.caller = (void *)this;
			msg.callback = (void *)callback;
			msg.userParam = userData;

			if (ClientIPC::getInstance().sendMessage(msg) == true)
			{
				result = SCARD_ERROR_OK;
			}
			else
			{
				_ERR("sendMessage failed");
				result = SCARD_ERROR_IPC_FAILED;
			}
#endif
		}
		else
		{
			_ERR("unavailable session");
			result = SCARD_ERROR_ILLEGAL_STATE;
		}

		return result;
	}

	Channel *Session::openChannelSync(int id, const ByteArray &aid)
		throw (ExceptionBase &, ErrorIO &, ErrorIllegalState &,
			ErrorIllegalParameter &, ErrorSecurity &)
	{
		Channel *channel = NULL;

		if (getReader()->isSecureElementPresent() == true)
		{
#ifdef USE_GDBUS
			gint ret;
			GVariant *var_aid = NULL, *var_response = NULL;
			guint channel_id;
			GError *error = NULL;

			var_aid = GDBusHelper::convertByteArrayToVariant(aid);

			if (smartcard_service_session_call_open_channel_sync(
				(SmartcardServiceSession *)proxy,
				GPOINTER_TO_UINT(context),
				GPOINTER_TO_UINT(handle),
				(guint)id, var_aid, &ret, &channel_id,
				&var_response, NULL, &error) == true) {
				if (ret == SCARD_ERROR_OK && channel_id != 0) {
					ByteArray response;

					GDBusHelper::convertVariantToByteArray(
						var_response, response);

					/* create new instance of channel */
					channel = new ClientChannel(context,
						this, channel_id,
						response, (void *)channel_id);
					if (channel != NULL)
					{
						channels.push_back(channel);
					}
					else
					{
						_ERR("alloc failed");

						THROW_ERROR(SCARD_ERROR_OUT_OF_MEMORY);
					}
				} else {
					_ERR("smartcard_service_session_call_open_channel_sync failed, [%d]", ret);

					THROW_ERROR(ret);
				}
			} else {
				_ERR("smartcard_service_session_call_open_channel_sync failed, [%s]", error->message);
				g_error_free(error);

				THROW_ERROR(SCARD_ERROR_IPC_FAILED);
			}
#else
			Message msg;
			int rv;

#ifdef CLIENT_IPC_THREAD
			/* request channel handle from server */
			msg.message = Message::MSG_REQUEST_OPEN_CHANNEL;
			msg.param1 = id;
			msg.param2 = (unsigned long)handle;
			msg.data = aid;
			msg.error = (unsigned long)context; /* using error to context */
			msg.caller = (void *)this;
			msg.callback = (void *)this; /* if callback is class instance, it means synchronized call */

			syncLock();
			if (ClientIPC::getInstance().sendMessage(msg) == true)
			{
				rv = waitTimedCondition(0);
				if (rv != 0)
				{
					_ERR("time over");
					this->error = SCARD_ERROR_OPERATION_TIMEOUT;
				}

				channel = openedChannel;
			}
			else
			{
				_ERR("sendMessage failed");
				this->error = SCARD_ERROR_IPC_FAILED;
			}
			syncUnlock();
#endif
			if (this->error != SCARD_ERROR_OK)
			{
				ThrowError::throwError(this->error);
			}
#endif
		}
		else
		{
			_ERR("unavailable session");

			throw ErrorIllegalState(SCARD_ERROR_UNAVAILABLE);
		}

		return (Channel *)channel;
	}

	int Session::openChannel(int id, const ByteArray &aid, openChannelCallback callback, void *userData)
	{
		int result;

		if (getReader()->isSecureElementPresent() == true)
		{
#ifdef USE_GDBUS
			GVariant *var_aid;

			CallbackParam *param = new CallbackParam();

			param->instance = this;
			param->callback = (void *)callback;
			param->user_param = userData;

			var_aid = GDBusHelper::convertByteArrayToVariant(aid);

			smartcard_service_session_call_open_channel(
				(SmartcardServiceSession *)proxy,
				GPOINTER_TO_UINT(context),
				GPOINTER_TO_UINT(handle),
				(guint)id, var_aid, NULL,
				&Session::session_open_channel_cb, param);
#else
			Message msg;

			/* request channel handle from server */
			msg.message = Message::MSG_REQUEST_OPEN_CHANNEL;
			msg.param1 = id;
			msg.param2 = (unsigned long)handle;
			msg.data = aid;
			msg.error = (unsigned long)context; /* using error to context */
			msg.caller = (void *)this;
			msg.callback = (void *)callback;
			msg.userParam = userData;

			if (ClientIPC::getInstance().sendMessage(msg) == true)
			{
				result = SCARD_ERROR_OK;
			}
			else
			{
				_ERR("sendMessage failed");
				result = SCARD_ERROR_IPC_FAILED;
			}
#endif
		}
		else
		{
			_ERR("unavailable session");
			result = SCARD_ERROR_ILLEGAL_STATE;
		}

		return result;
	}

	Channel *Session::openBasicChannelSync(const ByteArray &aid)
		throw (ErrorIO &, ErrorIllegalState &, ErrorIllegalParameter &, ErrorSecurity &)
	{
		return openChannelSync(0, aid);
	}

	Channel *Session::openBasicChannelSync(const unsigned char *aid, unsigned int length)
		throw (ErrorIO &, ErrorIllegalState &, ErrorIllegalParameter &, ErrorSecurity &)
	{
		ByteArray temp(aid, length);

		return openBasicChannelSync(temp);
	}

	int Session::openBasicChannel(const ByteArray &aid, openChannelCallback callback, void *userData)
	{
		return openChannel(0, aid, callback, userData);
	}

	int Session::openBasicChannel(const unsigned char *aid, unsigned int length,
		openChannelCallback callback, void *userData)
	{
		ByteArray temp(aid, length);

		return openBasicChannel(temp, callback, userData);
	}

	Channel *Session::openLogicalChannelSync(const ByteArray &aid)
		throw (ErrorIO &, ErrorIllegalState &, ErrorIllegalParameter &, ErrorSecurity &)
	{
		return openChannelSync(1, aid);
	}

	Channel *Session::openLogicalChannelSync(const unsigned char *aid, unsigned int length)
		throw (ErrorIO &, ErrorIllegalState &, ErrorIllegalParameter &, ErrorSecurity &)
	{
		ByteArray temp(aid, length);

		return openLogicalChannelSync(temp);
	}

	int Session::openLogicalChannel(const ByteArray &aid, openChannelCallback callback, void *userData)
	{
		return openChannel(1, aid, callback, userData);
	}

	int Session::openLogicalChannel(const unsigned char *aid, unsigned int length,
		openChannelCallback callback, void *userData)
	{
		ByteArray temp(aid, length);

		return openLogicalChannel(temp, callback, userData);
	}

#ifndef USE_GDBUS
	bool Session::dispatcherCallback(void *message)
	{
		Message *msg = (Message *)message;
		Session *session;
		bool result = false;

		if (msg == NULL)
		{
			_ERR("message is null");
			return result;
		}

		session = (Session *)msg->caller;

		switch (msg->message)
		{
		case Message::MSG_REQUEST_OPEN_CHANNEL :
			{
				Channel *channel = NULL;

				_INFO("MSG_REQUEST_OPEN_CHANNEL");

				if (msg->param1 != 0)
				{
					/* create new instance of channel */
					channel = new ClientChannel(session->context,
						session, msg->param2, msg->data, (void *)msg->param1);
					if (channel != NULL)
					{
						session->channels.push_back(channel);
					}
					else
					{
						_ERR("alloc failed");

						msg->error = SCARD_ERROR_OUT_OF_MEMORY;
					}
				}

				if (msg->isSynchronousCall() == true) /* synchronized call */
				{
					/* sync call */
					session->syncLock();

					/* copy result */
					session->error = msg->error;
					session->openedChannel = channel;

					session->signalCondition();
					session->syncUnlock();
				}
				else if (msg->callback != NULL)
				{
					openChannelCallback cb = (openChannelCallback)msg->callback;

					/* async call */
					cb(channel, msg->error, msg->userParam);
				}
			}
			break;

		case Message::MSG_REQUEST_GET_ATR :
			{
				_INFO("MSG_REQUEST_GET_ATR");

				if (msg->isSynchronousCall() == true) /* synchronized call */
				{
					/* sync call */
					session->syncLock();

					session->error = msg->error;
					session->atr = msg->data;

					session->signalCondition();
					session->syncUnlock();
				}
				else if (msg->callback != NULL)
				{
					getATRCallback cb = (getATRCallback)msg->callback;

					/* async call */
					cb(msg->data.getBuffer(),
						msg->data.size(),
						msg->error,
						msg->userParam);
				}
			}
			break;

		case Message::MSG_REQUEST_CLOSE_SESSION :
			{
				_INFO("MSG_REQUEST_CLOSE_SESSION");

				if (msg->isSynchronousCall() == true) /* synchronized call */
				{
					/* sync call */
					session->syncLock();

					session->error = msg->error;

					session->signalCondition();
					session->syncUnlock();
				}
				else if (msg->callback != NULL)
				{
					closeSessionCallback cb = (closeSessionCallback)msg->callback;

					/* async call */
					cb(msg->error, msg->userParam);
				}
			}
			break;

		case Message::MSG_REQUEST_GET_CHANNEL_COUNT :
			{
				_INFO("MSG_REQUEST_GET_CHANNEL_COUNT");

				if (msg->isSynchronousCall() == true) /* synchronized call */
				{
					/* sync call */
					session->syncLock();

					session->error = msg->error;
					session->channelCount = msg->param1;

					session->signalCondition();
					session->syncUnlock();
				}
				else if (msg->callback != NULL)
				{
					getChannelCountCallback cb = (getChannelCountCallback)msg->callback;

					/* async call */
					cb(msg->param1, msg->error, msg->userParam);
				}
			}
			break;

		default :
			_DBG("unknown message : %s", msg->toString().c_str());
			break;
		}

		delete msg;

		return result;
	}
#endif
} /* namespace smartcard_service_api */

/* export C API */
#define SESSION_EXTERN_BEGIN \
	if (handle != NULL) \
	{ \
		Session *session = (Session *)handle;

#define SESSION_EXTERN_END \
	} \
	else \
	{ \
		_ERR("Invalid param"); \
	}

using namespace smartcard_service_api;

EXTERN_API reader_h session_get_reader(session_h handle)
{
	reader_h reader = NULL;

	SESSION_EXTERN_BEGIN;
		reader = session->getReader();
	SESSION_EXTERN_END;

	return reader;
}

EXTERN_API int session_get_atr(session_h handle, session_get_atr_cb callback, void *userData)
{
	int result = -1;

	SESSION_EXTERN_BEGIN;
		result = session->getATR((getATRCallback)callback, userData);
	SESSION_EXTERN_END;

	return result;
}

EXTERN_API int session_close(session_h handle, session_close_session_cb callback, void *userData)
{
	int result = -1;

	SESSION_EXTERN_BEGIN;
		result = session->close((closeSessionCallback)callback, userData);
	SESSION_EXTERN_END;

	return result;
}

EXTERN_API bool session_is_closed(session_h handle)
{
	bool result = false;

	SESSION_EXTERN_BEGIN;
		result = session->isClosed();
	SESSION_EXTERN_END;

	return result;
}

EXTERN_API void session_close_channels(session_h handle)
{
	SESSION_EXTERN_BEGIN;
		session->closeChannels();
	SESSION_EXTERN_END;
}

EXTERN_API int session_open_basic_channel(session_h handle, unsigned char *aid,
	unsigned int length, session_open_channel_cb callback, void *userData)
{
	int result = -1;

	SESSION_EXTERN_BEGIN;
		result = session->openBasicChannel(aid, length, (openChannelCallback)callback, userData);
	SESSION_EXTERN_END;

	return result;
}

EXTERN_API int session_open_logical_channel(session_h handle, unsigned char *aid,
	unsigned int length, session_open_channel_cb callback, void *userData)
{
	int result = -1;

	SESSION_EXTERN_BEGIN;
		result = session->openLogicalChannel(aid, length, (openChannelCallback)callback, userData);
	SESSION_EXTERN_END;

	return result;
}

EXTERN_API int session_get_channel_count(session_h handle, session_get_channel_count_cb callback, void * userData)
{
	int result = -1;

	SESSION_EXTERN_BEGIN;
		result = session->getChannelCount((getChannelCountCallback)callback, userData);
	SESSION_EXTERN_END;

	return result;
}

EXTERN_API void session_destroy_instance(session_h handle)
{
}

EXTERN_API int session_get_atr_sync(session_h handle, unsigned char **buffer, unsigned int *length)
{
	ByteArray temp;
	int result = -1;

#ifdef CLIENT_IPC_THREAD
	if (buffer == NULL || length == NULL)
		return result;

	SESSION_EXTERN_BEGIN;
		temp = session->getATRSync();
		if (temp.size() > 0)
		{
			*length = temp.size();
			*buffer = (unsigned char *)calloc(1, *length);
			memcpy(*buffer, temp.getBuffer(), *length);

			result = 0;
		}
		SESSION_EXTERN_END;
#endif

	return result;
}

EXTERN_API void session_close_sync(session_h handle)
{
#ifdef CLIENT_IPC_THREAD
	SESSION_EXTERN_BEGIN;
		session->closeSync();
	SESSION_EXTERN_END;
#endif
}

EXTERN_API channel_h session_open_basic_channel_sync(session_h handle, unsigned char *aid, unsigned int length)
{
	channel_h result = NULL;

#ifdef CLIENT_IPC_THREAD
	SESSION_EXTERN_BEGIN;
		result = session->openBasicChannelSync(aid, length);
	SESSION_EXTERN_END;
#endif

	return result;
}

EXTERN_API channel_h session_open_logical_channel_sync(session_h handle, unsigned char *aid, unsigned int length)
{
	channel_h result = NULL;

#ifdef CLIENT_IPC_THREAD
	SESSION_EXTERN_BEGIN;
		result = session->openLogicalChannelSync(aid, length);
	SESSION_EXTERN_END;
#endif

	return result;
}

EXTERN_API unsigned int session_get_channel_count_sync(session_h handle)
{
	unsigned int result = 0;

#ifdef CLIENT_IPC_THREAD
	SESSION_EXTERN_BEGIN;
		result = session->getChannelCountSync();
	SESSION_EXTERN_END;
#endif

	return result;
}
