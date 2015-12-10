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
#include "ClientGDBus.h"

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
		gint result = SCARD_ERROR_UNKNOWN;
		gint channel_number;
		guint channel_id;
		GVariant *var_response;
		GError *error = NULL;
		Channel *channel = NULL;

		_INFO("MSG_REQUEST_OPEN_CHANNEL");

		if (param == NULL) {
			_ERR("null parameter!!!");
			return;
		}

		session = (Session *)param->instance;
		callback = (openChannelCallback)param->callback;

		if (smartcard_service_session_call_open_channel_finish(
			SMARTCARD_SERVICE_SESSION(source_object),
			&result, &channel_id, &channel_number, &var_response,
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

	const ByteArray Session::getATRSync()
		throw (ExceptionBase &, ErrorIO &, ErrorSecurity &,
			ErrorIllegalState &, ErrorIllegalParameter &)
	{
		ByteArray result;

		if (getReader()->isSecureElementPresent() == true)
		{
			if (atr.isEmpty() == true)
			{
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
		}
	}

	int Session::close(closeSessionCallback callback, void *userData)
	{
		int result = SCARD_ERROR_OK;

		if (isClosed() == false)
		{
			closed = true;
			closeChannels();

			CallbackParam *param = new CallbackParam();

			param->instance = this;
			param->callback = (void *)callback;
			param->user_param = userData;

			smartcard_service_session_call_close_session(
				(SmartcardServiceSession *)proxy,
				GPOINTER_TO_UINT(context),
				GPOINTER_TO_UINT(handle), NULL,
				&Session::session_close_cb, param);
		}

		return result;
	}

	size_t Session::getChannelCount() const
	{
		size_t count = 0;

		if (getReader()->isSecureElementPresent() == true)
		{
			count = channels.size();
		}
		else
		{
			_ERR("unavailable session");
			throw ErrorIllegalState(SCARD_ERROR_UNAVAILABLE);
		}

		return count;
	}
	Channel *Session::openChannelSync(int id, const ByteArray &aid)
		throw (ExceptionBase &, ErrorIO &, ErrorIllegalState &,
			ErrorIllegalParameter &, ErrorSecurity &)
	{
		return openChannelSync(id, aid, 0x00);
	}

	Channel *Session::openChannelSync(int id, const ByteArray &aid, unsigned char P2)
		throw (ExceptionBase &, ErrorIO &, ErrorIllegalState &,
			ErrorIllegalParameter &, ErrorSecurity &)
	{
		Channel *channel = NULL;

		if (getReader()->isSecureElementPresent() == true)
		{
			gint ret;
			GVariant *var_aid = NULL, *var_response = NULL;
			guint channel_id;
			gint channel_number;
			GError *error = NULL;

			var_aid = GDBusHelper::convertByteArrayToVariant(aid);

			if (smartcard_service_session_call_open_channel_sync(
				(SmartcardServiceSession *)proxy,
				GPOINTER_TO_UINT(context),
				GPOINTER_TO_UINT(handle),
				(guint)id, var_aid, (guint8)P2, &ret, &channel_id, &channel_number,
				&var_response, NULL, &error) == true) {
				if (ret == SCARD_ERROR_OK && channel_id != 0) {
					ByteArray response;

					GDBusHelper::convertVariantToByteArray(
						var_response, response);

					/* create new instance of channel */
					channel = new ClientChannel(context,
						this, channel_number,
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
				(guint)id, var_aid, 0, NULL,
				&Session::session_open_channel_cb, param);

			result = SCARD_ERROR_OK;
		}
		else
		{
			_ERR("unavailable session");
			result = SCARD_ERROR_ILLEGAL_STATE;
		}

		return result;
	}

	Channel *Session::openBasicChannelSync(const ByteArray &aid)
		throw (ExceptionBase &, ErrorIO &, ErrorIllegalState &,
			ErrorIllegalParameter &, ErrorSecurity &)
	{
		return openChannelSync(0, aid, 0x00);
	}

	Channel *Session::openBasicChannelSync(const ByteArray &aid, unsigned char P2)
		throw (ExceptionBase &, ErrorIO &, ErrorIllegalState &,
			ErrorIllegalParameter &, ErrorSecurity &)
	{
		return openChannelSync(0, aid, P2);
	}

	Channel *Session::openBasicChannelSync(const unsigned char *aid, unsigned int length)
		throw (ExceptionBase &, ErrorIO &, ErrorIllegalState &,
			ErrorIllegalParameter &, ErrorSecurity &)
	{
		ByteArray temp(aid, length);

		return openBasicChannelSync(temp, 0x00);
	}

	Channel *Session::openBasicChannelSync(const unsigned char *aid, unsigned int length, unsigned char P2)
		throw (ExceptionBase &, ErrorIO &, ErrorIllegalState &,
			ErrorIllegalParameter &, ErrorSecurity &)
	{
		ByteArray temp(aid, length);

		return openBasicChannelSync(temp, P2);
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
		throw (ExceptionBase &, ErrorIO &, ErrorIllegalState &,
			ErrorIllegalParameter &, ErrorSecurity &)
	{
		return openChannelSync(1, aid, 0x00);
	}

	Channel *Session::openLogicalChannelSync(const ByteArray &aid, unsigned char P2)
		throw (ExceptionBase &, ErrorIO &, ErrorIllegalState &,
			ErrorIllegalParameter &, ErrorSecurity &)
	{
		return openChannelSync(1, aid, P2);
	}

	Channel *Session::openLogicalChannelSync(const unsigned char *aid, unsigned int length)
		throw (ExceptionBase &, ErrorIO &, ErrorIllegalState &,
			ErrorIllegalParameter &, ErrorSecurity &)
	{
		ByteArray temp(aid, length);

		return openLogicalChannelSync(temp, 0x00);
	}

	Channel *Session::openLogicalChannelSync(const unsigned char *aid, unsigned int length, unsigned char P2)
		throw (ExceptionBase &, ErrorIO &, ErrorIllegalState &,
			ErrorIllegalParameter &, ErrorSecurity &)
	{
		ByteArray temp(aid, length);

		return openLogicalChannelSync(temp, P2);
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

EXTERN_API int session_get_reader(session_h handle, int* reader_handle)
{
	int result = SCARD_ERROR_OK;
	reader_h reader = NULL;

	SESSION_EXTERN_BEGIN;

	try
	{
		reader = session->getReader();
		*reader_handle = (long)reader;
	}
	catch (ExceptionBase &e)
	{
		_ERR("Error occur : %s\n", e.what());
		result = e.getErrorCode();
		*reader_handle = 0;
	}
	catch (...)
	{
		_ERR("Error occur : unknown error\n");
		result = SCARD_ERROR_UNKNOWN;
		*reader_handle = 0;
	}

	SESSION_EXTERN_END;

	return result;
}

EXTERN_API int session_is_closed(session_h handle, bool* is_closed)
{
	int result = SCARD_ERROR_OK;

	SESSION_EXTERN_BEGIN;

	try
	{
		*is_closed = session->isClosed();
	}
	catch (ExceptionBase &e)
	{
		_ERR("Error occur : %s\n", e.what());
		result = e.getErrorCode();
	}
	catch (...)
	{
		_ERR("Error occur : unknown error\n");
		result = SCARD_ERROR_UNKNOWN;
	}

	SESSION_EXTERN_END;

	return result;
}

EXTERN_API int session_close_channels(session_h handle)
{
	int result = SCARD_ERROR_OK;

	SESSION_EXTERN_BEGIN;

	try
	{
		session->closeChannels();
	}
	catch (ExceptionBase &e)
	{
		_ERR("Error occur : %s\n", e.what());
		result = e.getErrorCode();
	}
	catch (...)
	{
		_ERR("Error occur : unknown error\n");
		result = SCARD_ERROR_UNKNOWN;
	}

	SESSION_EXTERN_END;

	return result;
}

EXTERN_API int session_get_atr_sync(session_h handle, unsigned char **buffer, unsigned int *length)
{
	ByteArray temp;
	int result = SCARD_ERROR_OK;

	SESSION_EXTERN_BEGIN;

	try
	{
		temp = session->getATRSync();

		if (temp.size() > 0)
		{
			*buffer = (unsigned char *)calloc(temp.size(), sizeof(char));
			*length = temp.size();

			memcpy(*buffer, temp.getBuffer(), *length);
		}
	}
	catch (ErrorIllegalState &e)
	{
		_ERR("Error occur : %s\n", e.what());
		result = e.getErrorCode();

		if(result == SCARD_ERROR_OPERATION_NOT_SUPPORTED)
		{
			*length = 0;
			result = SCARD_ERROR_OK;
		}
	}
	catch (ExceptionBase &e)
	{
		_ERR("Error occur : %s\n", e.what());
		result = e.getErrorCode();
		*length = 0;
	}
	catch (...)
	{
		_ERR("Error occur : unknown error\n");
		result = SCARD_ERROR_UNKNOWN;
		*length = 0;
	}

	SESSION_EXTERN_END;

	return result;
}

EXTERN_API int session_close_sync(session_h handle)
{
	int result = SCARD_ERROR_OK;

	SESSION_EXTERN_BEGIN;

	try
	{
		session->closeSync();
	}
	catch (ExceptionBase &e)
	{
		_ERR("Error occur : %s\n", e.what());
		result = e.getErrorCode();
	}
	catch (...)
	{
		_ERR("Error occur : unknown error\n");
		result = SCARD_ERROR_UNKNOWN;
	}

	SESSION_EXTERN_END;

	return result;
}

EXTERN_API int session_open_basic_channel_sync(session_h handle, unsigned char *aid,
	unsigned int length, unsigned char P2, int* channel_handle)
{
	int result = SCARD_ERROR_OK;

	SESSION_EXTERN_BEGIN;

	try
	{
		*channel_handle = (long)session->openBasicChannelSync(aid, length, P2);
	}
	catch (ExceptionBase &e)
	{
		_ERR("Error occur : %s\n", e.what());
		result = e.getErrorCode();
		*channel_handle = 0;
	}
	catch (...)
	{
		_ERR("Error occur : unknown error\n");
		result = SCARD_ERROR_UNKNOWN;
		*channel_handle = 0;
	}

	SESSION_EXTERN_END;

	return result;
}

EXTERN_API  int session_open_logical_channel_sync(session_h handle, unsigned char *aid,
	unsigned int length, unsigned char P2, int* channel_handle)
{
	int result = SCARD_ERROR_OK;

	SESSION_EXTERN_BEGIN;

	try
	{
		*channel_handle = (long)session->openLogicalChannelSync(aid, length, P2);
	}
	catch (ExceptionBase &e)
	{
		_ERR("Error occur : %s\n", e.what());
		result = e.getErrorCode();
		*channel_handle = 0;
	}
	catch (...)
	{
		_ERR("Error occur : unknown error\n");
		result = SCARD_ERROR_UNKNOWN;
		*channel_handle = 0;
	}

	SESSION_EXTERN_END;

	return result;
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

EXTERN_API size_t session_get_channel_count(session_h handle)
{
	size_t result = 0;

	SESSION_EXTERN_BEGIN;
		result = session->getChannelCount();
	SESSION_EXTERN_END;

	return result;
}

EXTERN_API void session_destroy_instance(session_h handle)
{
}

