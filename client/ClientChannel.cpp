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

/* standard library header */
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#ifdef USE_GDBUS
#include <glib.h>
#endif

/* SLP library header */

/* local header */
#include "Debug.h"
#include "ClientChannel.h"
#include "ReaderHelper.h"
#include "APDUHelper.h"
#ifdef USE_GDBUS
#include "ClientGDBus.h"
#else
#include "Message.h"
#include "ClientIPC.h"
#endif

#ifndef EXTERN_API
#define EXTERN_API __attribute__((visibility("default")))
#endif

namespace smartcard_service_api
{
	ClientChannel::ClientChannel(void *context, Session *session,
		int channelNum, const ByteArray &selectResponse, void *handle)
		: Channel(session)
	{
		this->channelNum = -1;
		this->handle = NULL;
		this->context = NULL;

		if (handle == NULL)
		{
			_ERR("ClientIPC::getInstance() failed");

			return;
		}

		this->channelNum = channelNum;
		this->handle = handle;
		this->selectResponse = selectResponse;
		this->context = context;
#ifdef USE_GDBUS
		/* initialize client */
		if (!g_thread_supported())
		{
			g_thread_init(NULL);
		}

		g_type_init();

		/* init default context */
		GError *error = NULL;

		proxy = smartcard_service_channel_proxy_new_for_bus_sync(
			G_BUS_TYPE_SYSTEM, G_DBUS_PROXY_FLAGS_NONE,
			"org.tizen.SmartcardService",
			"/org/tizen/SmartcardService/Channel",
			NULL, &error);
		if (proxy == NULL)
		{
			_ERR("Can not create proxy : %s", error->message);
			g_error_free(error);
			return;
		}
#endif
	}

	ClientChannel::~ClientChannel()
	{
		closeSync();
	}

#ifdef USE_GDBUS
	void ClientChannel::channel_transmit_cb(GObject *source_object,
		GAsyncResult *res, gpointer user_data)
	{
		CallbackParam *param = (CallbackParam *)user_data;
		transmitCallback callback;
		gint result;
		GVariant *var_response;
		GError *error = NULL;
		ByteArray response;

		_INFO("MSG_REQUEST_TRANSMIT");

		if (param == NULL) {
			_ERR("null parameter!!!");
			return;
		}

		callback = (transmitCallback)param->callback;

		if (smartcard_service_channel_call_transmit_finish(
			SMARTCARD_SERVICE_CHANNEL(source_object),
			&result, &var_response, res, &error) == true) {
			if (result == SCARD_ERROR_OK) {
				GDBusHelper::convertVariantToByteArray(var_response, response);
			} else {
				_ERR("smartcard_service_channel_call_transmit failed, [%d]", result);
			}
		} else {
			_ERR("smartcard_service_channel_call_transmit failed, [%s]", error->message);
			g_error_free(error);

			result = SCARD_ERROR_IPC_FAILED;
		}

		if (callback != NULL) {
			callback(response.getBuffer(),
				response.size(),
				result, param->user_param);
		}

		delete param;
	}

	void ClientChannel::channel_close_cb(GObject *source_object,
		GAsyncResult *res, gpointer user_data)
	{
		CallbackParam *param = (CallbackParam *)user_data;
		ClientChannel *channel;
		closeChannelCallback callback;
		gint result;
		GError *error = NULL;

		_INFO("MSG_REQUEST_CLOSE_CHANNEL");

		if (param == NULL) {
			_ERR("null parameter!!!");
			return;
		}

		channel = (ClientChannel *)param->instance;
		callback = (closeChannelCallback)param->callback;

		if (smartcard_service_channel_call_close_channel_finish(
			SMARTCARD_SERVICE_CHANNEL(source_object),
			&result, res, &error) == true) {
			if (result == SCARD_ERROR_OK) {
				channel->channelNum = -1;
			} else {
				_ERR("smartcard_service_channel_call_close_channel failed, [%d]", result);
			}
		} else {
			_ERR("smartcard_service_channel_call_close_channel failed, [%s]", error->message);
			g_error_free(error);

			result = SCARD_ERROR_IPC_FAILED;
		}

		if (callback != NULL) {
			callback(result, param->user_param);
		}

		delete param;
	}
#endif
	void ClientChannel::closeSync()
		throw(ExceptionBase &, ErrorIO &, ErrorSecurity &,
			ErrorIllegalState &, ErrorIllegalParameter &)
	{
		if (isClosed() == false)
		{
			if (getSession()->getReader()->isSecureElementPresent() == true)
			{
#ifdef USE_GDBUS
				gint ret;
				GError *error = NULL;

				if (proxy == NULL) {
					_ERR("dbus proxy is not initialized yet");
					throw ErrorIllegalState(SCARD_ERROR_NOT_INITIALIZED);
				}

				if (smartcard_service_channel_call_close_channel_sync(
					(SmartcardServiceChannel *)proxy,
					GPOINTER_TO_UINT(context),
					GPOINTER_TO_UINT(handle),
					&ret, NULL, &error) == true) {
					if (ret != SCARD_ERROR_OK) {
						_ERR("smartcard_service_channel_call_close_channel_sync failed, [%d]", ret);
						THROW_ERROR(ret);
					}
				} else {
					_ERR("smartcard_service_channel_call_close_channel_sync failed, [%s]", error->message);
					g_error_free(error);

					throw ErrorIO(SCARD_ERROR_IPC_FAILED);
				}
#else
				Message msg;
				int rv;

#ifdef CLIENT_IPC_THREAD
				/* send message to server */
				msg.message = Message::MSG_REQUEST_CLOSE_CHANNEL;
				msg.param1 = (unsigned long)handle;
				msg.error = (unsigned long)context; /* using error to context */
				msg.caller = (void *)this;
				msg.callback = (void *)this; /* if callback is class instance, it means synchronized call */

				syncLock();
				if (ClientIPC::getInstance().sendMessage(msg) == true)
				{
					rv = waitTimedCondition(0);
					if (rv < 0)
					{
						_ERR("timeout [%d]", rv);
						this->error = SCARD_ERROR_OPERATION_TIMEOUT;
					}
				}
				else
				{
					_ERR("sendMessage failed");
					this->error = SCARD_ERROR_IPC_FAILED;
				}
				syncUnlock();

				channelNum = -1;

				if (this->error != SCARD_ERROR_OK)
				{
					ThrowError::throwError(this->error);
				}
#endif
#endif
			}
			else
			{
				_INFO("unavailable channel");
			}
		}
	}

	int ClientChannel::close(closeChannelCallback callback, void *userParam)
	{
		int result = SCARD_ERROR_OK;

		if (isClosed() == false)
		{
			if (getSession()->getReader()->isSecureElementPresent() == true)
			{
#ifdef USE_GDBUS
				CallbackParam *param = new CallbackParam();

				param->instance = this;
				param->callback = (void *)callback;
				param->user_param = userParam;

				smartcard_service_channel_call_close_channel(
					(SmartcardServiceChannel *)proxy,
					GPOINTER_TO_UINT(context),
					GPOINTER_TO_UINT(handle), NULL,
					&ClientChannel::channel_close_cb, param);
#else
				Message msg;
				channelNum = -1;

				/* send message to server */
				msg.message = Message::MSG_REQUEST_CLOSE_CHANNEL;
				msg.param1 = (unsigned long)handle;
				msg.error = (unsigned long)context; /* using error to context */
				msg.caller = (void *)this;
				msg.callback = (void *)callback;
				msg.userParam = userParam;

				if (ClientIPC::getInstance().sendMessage(msg) == false)
				{
					_ERR("sendMessage failed");
					result = SCARD_ERROR_IPC_FAILED;
				}
#endif
			}
			else
			{
				_ERR("unavailable channel");
				result = SCARD_ERROR_ILLEGAL_STATE;
			}
		}

		return result;
	}

	int ClientChannel::transmitSync(const ByteArray &command, ByteArray &result)
		throw(ExceptionBase &, ErrorIO &, ErrorIllegalState &,
			ErrorIllegalParameter &, ErrorSecurity &)
	{
		int rv = SCARD_ERROR_OK;

		if (getSession()->getReader()->isSecureElementPresent() == true)
		{
#ifdef USE_GDBUS
			GVariant *var_command = NULL, *var_response = NULL;
			GError *error = NULL;

			var_command = GDBusHelper::convertByteArrayToVariant(command);

			if (smartcard_service_channel_call_transmit_sync(
				(SmartcardServiceChannel *)proxy,
				GPOINTER_TO_UINT(context),
				GPOINTER_TO_UINT(handle),
				var_command, &rv, &var_response,
				NULL, &error) == true) {

				if (rv == SCARD_ERROR_OK) {
					GDBusHelper::convertVariantToByteArray(var_response, result);
				} else {
					_ERR("smartcard_service_session_call_get_atr_sync failed, [%d]", rv);
					THROW_ERROR(rv);
				}
			} else {
				_ERR("smartcard_service_session_call_get_atr_sync failed, [%s]", error->message);
				g_error_free(error);

				throw ErrorIO(SCARD_ERROR_IPC_FAILED);
			}
#else
			Message msg;
#ifdef CLIENT_IPC_THREAD
			/* send message to server */
			msg.message = Message::MSG_REQUEST_TRANSMIT;
			msg.param1 = (unsigned long)handle;
			msg.param2 = 0;
			msg.data = command;
			msg.error = (unsigned long)context; /* using error to context */
			msg.caller = (void *)this;
			msg.callback = (void *)this; /* if callback is class instance, it means synchronized call */

			syncLock();
			if (ClientIPC::getInstance().sendMessage(msg) == true)
			{
				rv = waitTimedCondition(0);
				if (rv >= 0)
				{
					result = response;
					rv = SCARD_ERROR_OK;
				}
				else
				{
					_ERR("timeout [%d]", rv);
					this->error = SCARD_ERROR_OPERATION_TIMEOUT;
				}
			}
			else
			{
				_ERR("sendMessage failed");
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
			_ERR("unavailable channel");
			throw ErrorIllegalState(SCARD_ERROR_UNAVAILABLE);
		}

		return rv;
	}

	int ClientChannel::transmit(const ByteArray &command, transmitCallback callback, void *userParam)
	{
		int result;

		if (getSession()->getReader()->isSecureElementPresent() == true)
		{
#ifdef USE_GDBUS
			GVariant *var_command;
			CallbackParam *param = new CallbackParam();

			param->instance = this;
			param->callback = (void *)callback;
			param->user_param = userParam;

			var_command = GDBusHelper::convertByteArrayToVariant(command);

			smartcard_service_channel_call_transmit(
				(SmartcardServiceChannel *)proxy,
				GPOINTER_TO_UINT(context),
				GPOINTER_TO_UINT(handle),
				var_command, NULL,
				&ClientChannel::channel_transmit_cb, param);

			result = SCARD_ERROR_OK;
#else
			Message msg;

			/* send message to server */
			msg.message = Message::MSG_REQUEST_TRANSMIT;
			msg.param1 = (unsigned long)handle;
			msg.param2 = 0;
			msg.data = command;
			msg.error = (unsigned long)context; /* using error to context */
			msg.caller = (void *)this;
			msg.callback = (void *)callback;
			msg.userParam = userParam;

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
			_ERR("unavailable channel");
			result = SCARD_ERROR_ILLEGAL_STATE;
		}

		return result;
	}

#ifndef USE_GDBUS
	bool ClientChannel::dispatcherCallback(void *message)
	{
		Message *msg = (Message *)message;
		ClientChannel *channel = NULL;
		bool result = false;

		if (msg == NULL)
		{
			_ERR("message is null");
			return result;
		}

		channel = (ClientChannel *)msg->caller;

		switch (msg->message)
		{
		case Message::MSG_REQUEST_TRANSMIT :
			{
				/* transmit result */
				_INFO("MSG_REQUEST_TRANSMIT");

				if (msg->isSynchronousCall() == true) /* synchronized call */
				{
					/* sync call */
					channel->syncLock();

					/* copy result */
					channel->error = msg->error;
					channel->response = msg->data;

					channel->signalCondition();
					channel->syncUnlock();
				}
				else if (msg->callback != NULL)
				{
					transmitCallback cb = (transmitCallback)msg->callback;

					/* async call */
					cb(msg->data.getBuffer(),
						msg->data.size(),
						msg->error,
						msg->userParam);
				}
			}
			break;

		case Message::MSG_REQUEST_CLOSE_CHANNEL :
			{
				_INFO("MSG_REQUEST_CLOSE_CHANNEL");

				if (msg->isSynchronousCall() == true) /* synchronized call */
				{
					/* sync call */
					channel->syncLock();

					channel->error = msg->error;

					channel->signalCondition();
					channel->syncUnlock();
				}
				else if (msg->callback != NULL)
				{
					closeChannelCallback cb = (closeChannelCallback)msg->callback;

					/* async call */
					cb(msg->error, msg->userParam);
				}
			}
			break;

		default:
			_DBG("Unknown message : %s", msg->toString().c_str());
			break;
		}

		delete msg;

		return result;
	}
#endif /* USE_GDBUS */
} /* namespace smartcard_service_api */

/* export C API */
#define CHANNEL_EXTERN_BEGIN \
	if (handle != NULL) \
	{ \
		ClientChannel *channel = (ClientChannel *)handle;

#define CHANNEL_EXTERN_END \
	} \
	else \
	{ \
		_ERR("Invalid param"); \
	}

using namespace smartcard_service_api;

EXTERN_API int channel_close(channel_h handle, channel_close_cb callback, void *userParam)
{
	int result = -1;

	CHANNEL_EXTERN_BEGIN;
	result = channel->close((closeChannelCallback)callback, userParam);
	CHANNEL_EXTERN_END;

	return result;
}

EXTERN_API int channel_transmit(channel_h handle, unsigned char *command,
	unsigned int length, channel_transmit_cb callback, void *userParam)
{
	int result = -1;

	CHANNEL_EXTERN_BEGIN;
	ByteArray temp;

	temp.assign(command, length);
	result = channel->transmit(temp, (transmitCallback)callback, userParam);
	CHANNEL_EXTERN_END;

	return result;
}

EXTERN_API void channel_close_sync(channel_h handle)
{
#ifdef CLIENT_IPC_THREAD
	CHANNEL_EXTERN_BEGIN;
	try
	{
		channel->closeSync();
	}
	catch (...)
	{
	}
	CHANNEL_EXTERN_END;
#endif
}

EXTERN_API int channel_transmit_sync(channel_h handle, unsigned char *command,
	unsigned int cmd_len, unsigned char **response, unsigned int *resp_len)
{
	int result = -1;

#ifdef CLIENT_IPC_THREAD
	if (command == NULL || cmd_len == 0 || response == NULL || resp_len == NULL)
		return result;

	CHANNEL_EXTERN_BEGIN;
	ByteArray temp, resp;

	temp.assign(command, cmd_len);

	try
	{
		result = channel->transmitSync(temp, resp);
		if (resp.size() > 0)
		{
			*resp_len = resp.size();
			*response = (unsigned char *)calloc(1, *resp_len);
			memcpy(*response, resp.getBuffer(), *resp_len);
		}
	}
	catch (...)
	{
		result = -1;
	}
	CHANNEL_EXTERN_END;
#endif

	return result;
}

EXTERN_API bool channel_is_basic_channel(channel_h handle)
{
	bool result = false;

	CHANNEL_EXTERN_BEGIN;
	result = channel->isBasicChannel();
	CHANNEL_EXTERN_END;

	return result;
}

EXTERN_API bool channel_is_closed(channel_h handle)
{
	bool result = false;

	CHANNEL_EXTERN_BEGIN;
	result = channel->isClosed();
	CHANNEL_EXTERN_END;

	return result;
}

EXTERN_API unsigned int channel_get_select_response_length(channel_h handle)
{
	unsigned int result = 0;

	CHANNEL_EXTERN_BEGIN;
	result = channel->getSelectResponse().size();
	CHANNEL_EXTERN_END;

	return result;
}

EXTERN_API bool channel_get_select_response(channel_h handle,
	unsigned char *buffer, unsigned int length)
{
	bool result = false;

	if (buffer == NULL || length == 0)
	{
		return result;
	}

	CHANNEL_EXTERN_BEGIN;
	ByteArray response;

	response = channel->getSelectResponse();
	if (response.size() > 0)
	{
		memcpy(buffer, response.getBuffer(), MIN(length, response.size()));
		result = true;
	}
	CHANNEL_EXTERN_END;

	return result;
}

EXTERN_API session_h channel_get_session(channel_h handle)
{
	session_h session = NULL;

	CHANNEL_EXTERN_BEGIN;
	session = channel->getSession();
	CHANNEL_EXTERN_END;

	return session;
}

EXTERN_API void channel_destroy_instance(channel_h handle)
{
	/* do nothing */
}
