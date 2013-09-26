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

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "Debug.h"
#include "Reader.h"
#include "Session.h"
#include "ClientGDBus.h"

#ifndef EXTERN_API
#define EXTERN_API __attribute__((visibility("default")))
#endif

namespace smartcard_service_api
{
	Reader::Reader(void *context, const char *name, void *handle) :
		ReaderHelper(name), context(context), handle(handle)
	{
		_BEGIN();

		if (context == NULL || handle == NULL)
		{
			_ERR("invalid param");

			return;
		}

		/* init default context */
		GError *error = NULL;

		proxy = smartcard_service_reader_proxy_new_for_bus_sync(
			G_BUS_TYPE_SYSTEM, G_DBUS_PROXY_FLAGS_NONE,
			"org.tizen.SmartcardService",
			"/org/tizen/SmartcardService/Reader",
			NULL, &error);
		if (proxy == NULL)
		{
			_ERR("Can not create proxy : %s", error->message);
			g_error_free(error);
			return;
		}

		present = true;

		_END();
	}

	Reader::~Reader()
	{
		size_t i;

		closeSessions();

		for (i = 0; i < sessions.size(); i++)
		{
			delete (Session *)sessions[i];
		}

		sessions.clear();
	}

	void Reader::closeSessions()
		throw(ErrorIO &, ErrorIllegalState &)
	{
		size_t i;

		for (i = 0; i < sessions.size(); i++)
		{
			sessions[i]->closeSync();
		}
	}

	SessionHelper *Reader::openSessionSync()
		throw(ExceptionBase &, ErrorIO &, ErrorIllegalState &,
			ErrorIllegalParameter &, ErrorSecurity &)
	{
		Session *session = NULL;

		if (isSecureElementPresent() == true)
		{
			gint result;
			GError *error = NULL;
			guint session_id;

			if (smartcard_service_reader_call_open_session_sync(
				(SmartcardServiceReader *)proxy,
				GPOINTER_TO_UINT(context),
				GPOINTER_TO_UINT(handle),
				&result, &session_id, NULL, &error) == true) {
				if (result == SCARD_ERROR_OK) {
					/* create new instance of channel */
					session = new Session(context, this,
						GUINT_TO_POINTER(session_id));
					if (session != NULL) {
						sessions.push_back(session);
					} else {
						_ERR("Session creating instance failed");

						THROW_ERROR(SCARD_ERROR_OUT_OF_MEMORY);
					}
				} else {
					_ERR("smartcard_service_reader_call_open_session_sync failed, [%d]", result);

					THROW_ERROR(result);
				}
			} else {
				_ERR("smartcard_service_reader_call_open_session_sync failed, [%s]", error->message);
				g_error_free(error);

				THROW_ERROR(SCARD_ERROR_IPC_FAILED);
			}
		}
		else
		{
			_ERR("unavailable reader");
			throw ErrorIllegalState(SCARD_ERROR_UNAVAILABLE);
		}

		return session;
	}

	void Reader::reader_open_session_cb(GObject *source_object,
		GAsyncResult *res, gpointer user_data)
	{
		CallbackParam *param = (CallbackParam *)user_data;
		Reader *reader;
		openSessionCallback callback;
		Session *session = NULL;
		gint result;
		guint handle;
		GError *error = NULL;

		_INFO("MSG_REQUEST_OPEN_SESSION");

		if (param == NULL) {
			_ERR("null parameter!!!");
			return;
		}

		reader = (Reader *)param->instance;
		callback = (openSessionCallback)param->callback;

		if (smartcard_service_reader_call_open_session_finish(
			SMARTCARD_SERVICE_READER(source_object),
			&result, &handle, res, &error) == true) {
			if (result == SCARD_ERROR_OK) {
				/* create new instance of channel */
				session = new Session(reader->context, reader,
					GUINT_TO_POINTER(handle));
				if (session != NULL) {
					reader->sessions.push_back(session);
				} else {
					_ERR("Session creating instance failed");

					result = SCARD_ERROR_OUT_OF_MEMORY;
				}
			} else {
				_ERR("smartcard_service_reader_call_open_session failed, [%d]", result);
			}
		} else {
			_ERR("smartcard_service_reader_call_open_session failed, [%s]", error->message);
			g_error_free(error);

			result = SCARD_ERROR_IPC_FAILED;
		}

		if (callback != NULL) {
			callback(session, result, param->user_param);
		}

		delete param;
	}
	int Reader::openSession(openSessionCallback callback, void *userData)
	{
		int result;

		_BEGIN();

		if (isSecureElementPresent() == true)
		{
			CallbackParam *param = new CallbackParam();

			param->instance = this;
			param->callback = (void *)callback;
			param->user_param = userData;

			smartcard_service_reader_call_open_session(
				(SmartcardServiceReader *)proxy,
				GPOINTER_TO_UINT(context),
				GPOINTER_TO_UINT(handle),
				NULL, &Reader::reader_open_session_cb, param);

			result = SCARD_ERROR_OK;
		}
		else
		{
			_ERR("unavailable reader");
			result = SCARD_ERROR_ILLEGAL_STATE;
		}

		_END();

		return result;
	}
} /* namespace smartcard_service_api */

/* export C API */
#define READER_EXTERN_BEGIN \
	if (handle != NULL) \
	{ \
		Reader *reader = (Reader *)handle;

#define READER_EXTERN_END \
	} \
	else \
	{ \
		_ERR("Invalid param"); \
	}

using namespace smartcard_service_api;

EXTERN_API const char *reader_get_name(reader_h handle)
{
	const char *name = NULL;

	READER_EXTERN_BEGIN;
	name = reader->getName();
	READER_EXTERN_END;

	return name;
}

EXTERN_API se_service_h reader_get_se_service(reader_h handle)
{
	se_service_h service = NULL;

	READER_EXTERN_BEGIN;
	service = (se_service_h)reader->getSEService();
	READER_EXTERN_END;

	return service;
}

EXTERN_API bool reader_is_secure_element_present(reader_h handle)
{
	bool result = false;

	READER_EXTERN_BEGIN;
	result = reader->isSecureElementPresent();
	READER_EXTERN_END;

	return result;
}

EXTERN_API int reader_open_session(reader_h handle, reader_open_session_cb callback, void *userData)
{
	int result = -1;

	READER_EXTERN_BEGIN;
	result = reader->openSession((openSessionCallback)callback, userData);
	READER_EXTERN_END;

	return result;
}

EXTERN_API session_h reader_open_session_sync(reader_h handle)
{
	session_h result = NULL;

	READER_EXTERN_BEGIN;
	result = (session_h)reader->openSessionSync();
	READER_EXTERN_END;

	return result;
}

EXTERN_API void reader_close_sessions(reader_h handle)
{
	READER_EXTERN_BEGIN;
	reader->closeSessions();
	READER_EXTERN_END;
}

EXTERN_API void reader_destroy_instance(reader_h handle)
{
}
