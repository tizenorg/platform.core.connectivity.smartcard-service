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
#include <unistd.h>
#include <string.h>
#include <glib.h>
#include <glib-object.h>

/* local header */
#include "Debug.h"
#include "SEService.h"
#include "ClientChannel.h"
#include "Reader.h"
#include "ClientGDBus.h"

#ifndef EXTERN_API
#define EXTERN_API __attribute__((visibility("default")))
#endif

#define SHUTDOWN_DELAY		500000 /* us */
#define VERSION "3.0"

namespace smartcard_service_api
{
	SEService::SEService() : SEServiceHelper(),
		handle(-1), context(NULL), handler(NULL), listener(NULL),
		version(VERSION)
	{
		proxy = NULL;
	}

	SEService::SEService(void *user_data, serviceConnected handler)
		throw(ErrorIO &, ErrorIllegalParameter &) :
		SEServiceHelper(), handle(-1),
		listener(NULL), version(VERSION)
	{
		initialize(user_data, handler);
	}

	SEService::SEService(void *user_data, SEServiceListener *listener)
		throw(ErrorIO &, ErrorIllegalParameter &) :
		SEServiceHelper(), handle(-1),
		handler(NULL), version(VERSION)
	{
		initialize(user_data, listener);
	}

	SEService::SEService(void *user_data)
		throw(ErrorIO &, ErrorIllegalParameter &, ExceptionBase &) :
		SEServiceHelper(), handle(-1),
		handler(NULL), version(VERSION)
	{
		initializeSync(user_data);
	}

	SEService::~SEService()
	{
		try
		{
			size_t i;

			shutdownSync();

			for (i = 0; i < readers.size(); i++)
			{
				delete (Reader *)readers[i];
			}

			readers.clear();
		}
		catch (ExceptionBase &e)
		{
			_ERR("EXCEPTION : %s", e.what());
		}
		catch (...)
		{
			_ERR("EXCEPTION!!!");
		}
	}

	SEService *SEService::createInstance(void *user_data,
		SEServiceListener *listener)
		throw(ErrorIO &, ErrorIllegalParameter &)
	{
		return new SEService(user_data, listener);
	}

	SEService *SEService::createInstance(void *user_data,
		serviceConnected handler)
		throw(ErrorIO &, ErrorIllegalParameter &)
	{
		return new SEService(user_data, handler);
	}

	void SEService::reader_inserted(GObject *source_object,
		guint reader_id, gchar *reader_name, gpointer user_data)
	{
		Reader *reader = NULL;
		SEService *service = (SEService *)user_data;

		_INFO("[MSG_NOTIFY_SE_INSERTED]");

		/* add readers */
		reader = new Reader(service->context,
			reader_name, GUINT_TO_POINTER(reader_id));
		if (reader != NULL)
		{
			service->readers.push_back(reader);
		}
		else
		{
			_ERR("alloc failed");
		}

		if (service->listener != NULL)
		{
			service->listener->eventHandler(service,
				reader_name, 1, service->context);
		}
		else
		{
			_DBG("listener is null");
		}
	}

	void SEService::reader_removed(GObject *source_object,
		guint reader_id, gchar *reader_name, gpointer user_data)
	{
		SEService *service = (SEService *)user_data;
		size_t i;

		_INFO("[MSG_NOTIFY_SE_REMOVED]");

		for (i = 0; i < service->readers.size(); i++)
		{
			if (((Reader *)service->readers[i])->handle ==
				GUINT_TO_POINTER(reader_id))
			{
				((Reader *)service->readers[i])->unavailable();
				break;
			}
		}

		if (service->listener != NULL)
		{
			service->listener->eventHandler(service,
				reader_name, 2, service->context);
		}
		else
		{
			_DBG("listener is null");
		}
	}

	void SEService::se_service_shutdown_cb(GObject *source_object,
		GAsyncResult *res, gpointer user_data)
	{
		SEService *service = (SEService *)user_data;
		gint result;
		GError *error = NULL;

		if (smartcard_service_se_service_call_shutdown_finish(
			SMARTCARD_SERVICE_SE_SERVICE(source_object),
			&result, res, &error) == true) {
			if (result == SCARD_ERROR_OK) {
				service->connected = false;
			} else {
				_ERR("smartcard_service_se_service_call_shutdown failed, [%d]", result);
			}
		} else {
			_ERR("smartcard_service_se_service_call_shutdown failed, [%s]", error->message);
			g_error_free(error);
		}
	}

	void SEService::se_service_cb(GObject *source_object,
		GAsyncResult *res, gpointer user_data)
	{
		SEService *service = (SEService *)user_data;
		gint result;
		guint handle;
		GVariant *readers = NULL;
		GError *error = NULL;

		if (service == NULL) {
			_ERR("null parameter!!!");
			return;
		}

		if (smartcard_service_se_service_call_se_service_finish(
			SMARTCARD_SERVICE_SE_SERVICE(source_object),
			&result, &handle, &readers, res, &error) == true) {
			if (result == SCARD_ERROR_OK) {
				service->connected = true;
				service->handle = handle;
				service->parseReaderInformation(readers);
			}
		} else {
			_ERR("smartcard_service_se_service_call_se_service failed, [%s]", error->message);
			g_error_free(error);

			result = SCARD_ERROR_IPC_FAILED;
		}

		if (service->handler != NULL) {
			service->handler(service, service->context);
		} else if (service->listener != NULL) {
			if (result == SCARD_ERROR_OK) {
				service->listener->serviceConnected(service, service->context);
			} else {
				service->listener->errorHandler(service, result, service->context);
			}
		}
	}

	void SEService::shutdown()
	{
		shutdownSync();
	}

	void SEService::shutdownSync()
	{
		if (connected == true)
		{
			uint32_t i;

			for (i = 0; i < readers.size(); i++)
			{
				readers[i]->closeSessions();
			}

			gint result;
			GError *error = NULL;

			if (smartcard_service_se_service_call_shutdown_sync(
				(SmartcardServiceSeService *)proxy,
				handle,
				&result,
				NULL,
				&error) == false) {
				_ERR("smartcard_service_se_service_call_shutdown_sync failed, [%s]", error->message);

				g_error_free(error);
			}

			/* wait at least 500ms */
			usleep(SHUTDOWN_DELAY);

			connected = false;
		}
	}

	bool SEService::_initialize() throw(ErrorIO &)
	{
		bool result = false;

		_BEGIN();

		/* init default context */
		GError *error = NULL;

		proxy = smartcard_service_se_service_proxy_new_for_bus_sync(
			G_BUS_TYPE_SYSTEM, G_DBUS_PROXY_FLAGS_NONE,
			"org.tizen.SmartcardService",
			"/org/tizen/SmartcardService/SeService",
			NULL, &error);
		if (proxy == NULL)
		{
			_ERR("Can not create proxy : %s", error->message);
			g_error_free(error);
			return false;
		}

		g_signal_connect(proxy, "reader-inserted",
				G_CALLBACK(&SEService::reader_inserted), this);

		g_signal_connect(proxy, "reader-removed",
				G_CALLBACK(&SEService::reader_removed), this);

		/* request reader */
		smartcard_service_se_service_call_se_service(
			(SmartcardServiceSeService *)proxy,
			NULL,
			&SEService::se_service_cb,
			this);

		_END();

		return result;
	}

	int SEService::_initialize_sync_do_not_throw_exception()
	{
		gint result;
		guint handle;
		GError *error = NULL;
		GVariant *readers = NULL;
		SEService *service = (SEService *)this;

		_BEGIN();

		/* init default context */

		proxy = smartcard_service_se_service_proxy_new_for_bus_sync(
			G_BUS_TYPE_SYSTEM, G_DBUS_PROXY_FLAGS_NONE,
			"org.tizen.SmartcardService",
			"/org/tizen/SmartcardService/SeService",
			NULL, &error);
		if (proxy == NULL)
		{
			_ERR("Can not create proxy : %s", error->message);
			g_error_free(error);
			return false;
		}

		g_signal_connect(proxy, "reader-inserted",
				G_CALLBACK(&SEService::reader_inserted), this);

		g_signal_connect(proxy, "reader-removed",
				G_CALLBACK(&SEService::reader_removed), this);

		/* request reader */
		if(smartcard_service_se_service_call_se_service_sync(
			(SmartcardServiceSeService *)proxy, &result, &handle, &readers, NULL, &error) == true)
		{
			if (result == SCARD_ERROR_OK)
			{
				service->connected = true;
				service->handle = handle;
				service->parseReaderInformation(readers);
			}
			else
			{
				_ERR("Initialize error : %d", result);
			}
		}

		if (service->handler != NULL) {
			service->handler(service, service->context);
		} else if (service->listener != NULL) {
			if (result == SCARD_ERROR_OK) {
				service->listener->serviceConnected(service, service->context);
			} else {
				service->listener->errorHandler(service, result, service->context);
			}
		}

		_END();

		return result;
	}

	int SEService::_initialize_sync() throw(ErrorIO &, ExceptionBase &)
	{
		gint result;
		guint handle;
		GError *error = NULL;
		GVariant *readers = NULL;
		SEService *service = (SEService *)this;

		_BEGIN();

		/* init default context */

		proxy = smartcard_service_se_service_proxy_new_for_bus_sync(
			G_BUS_TYPE_SYSTEM, G_DBUS_PROXY_FLAGS_NONE,
			"org.tizen.SmartcardService",
			"/org/tizen/SmartcardService/SeService",
			NULL, &error);
		if (proxy == NULL)
		{
			_ERR("Can not create proxy : %s", error->message);
			g_error_free(error);
			return false;
		}

		g_signal_connect(proxy, "reader-inserted",
				G_CALLBACK(&SEService::reader_inserted), this);

		g_signal_connect(proxy, "reader-removed",
				G_CALLBACK(&SEService::reader_removed), this);

		/* request reader */
		if(smartcard_service_se_service_call_se_service_sync(
			(SmartcardServiceSeService *)proxy, &result, &handle, &readers, NULL, &error) == true)
		{
			if (result == SCARD_ERROR_OK)
			{
				service->connected = true;
				service->handle = handle;
				service->parseReaderInformation(readers);
			}
			else
			{
				throw ExceptionBase(result);
			}
		}
		else
		{
			_ERR("smartcard_service_se_service_call_se_service failed, [%s]", error->message);
			g_error_free(error);

			result = SCARD_ERROR_IPC_FAILED;
		}

		_END();

		return result;
	}

	bool SEService::initialize(void *context, serviceConnected handler)
		throw(ErrorIO &, ErrorIllegalParameter &)
	{
		if (context == NULL)
		{
			throw ErrorIllegalParameter(SCARD_ERROR_ILLEGAL_PARAM);
		}

		this->context = context;
		this->handler = handler;

		return _initialize();
	}

	bool SEService::initialize(void *context, SEServiceListener *listener)
		throw(ErrorIO &, ErrorIllegalParameter &)
	{
		if (context == NULL)
		{
			throw ErrorIllegalParameter(SCARD_ERROR_ILLEGAL_PARAM);
		}

		this->context = context;
		this->listener = listener;

		return _initialize_sync_do_not_throw_exception();
	}

	bool SEService::initializeSync(void *context)
		throw(ErrorIO &, ErrorIllegalParameter &, ExceptionBase &)
	{
		this->context = context;

		_initialize_sync();
		return true;
	}

	bool SEService::parseReaderInformation(GVariant *variant)
	{
		Reader *reader = NULL;

		GVariantIter *iter;
		guint handle;
		gchar *name;

		g_variant_get(variant, "a(us)", &iter);

		while (g_variant_iter_loop(iter, "(us)", &handle, &name) == true)
		{
			SECURE_LOGD("Reader : name [%s], handle [%08x]", name, handle);

			/* add readers */
			reader = new Reader((void *)this->handle, name, GUINT_TO_POINTER(handle));
			if (reader == NULL)
			{
				_ERR("alloc failed");
				continue;
			}

			readers.push_back(reader);
		}

		g_variant_iter_free(iter);

		return true;
	}

	bool SEService::parseReaderInformation(unsigned int count,
		const ByteArray &data)
	{
		size_t i;
		unsigned int offset = 0;
		unsigned int len = 0;
		void *handle = NULL;
		Reader *reader = NULL;
		char name[100];
		const uint8_t *buffer = NULL;


		for (i = 0; i < count && offset < data.size(); i++)
		{
			memset(name, 0, sizeof(name));

			buffer = data.getBuffer(offset);
			if(buffer == NULL)
				continue;

			memcpy(&len, buffer, sizeof(len));
			offset += sizeof(len);

			buffer = data.getBuffer(offset);
			if(buffer == NULL)
				return false;

			memcpy(name, buffer, len);
			offset += len;

			buffer = data.getBuffer(offset);
			if(buffer == NULL)
				return false;

			memcpy(&handle, buffer, sizeof(handle));
			offset += sizeof(handle);

			SECURE_LOGD("Reader [%d] : name [%s], handle [%p]", i, name, handle);

			/* add readers */
			reader = new Reader(context, name, handle);
			if (reader == NULL)
			{
				_ERR("alloc failed");
				continue;
			}

			readers.push_back(reader);
		}

		return true;
	}
} /* namespace smartcard_service_api */

/* export C API */
#define SE_SERVICE_EXTERN_BEGIN \
	if (handle != NULL) \
	{ \
		SEService *service = (SEService *)handle;

#define SE_SERVICE_EXTERN_END \
	} \
	else \
	{ \
		_ERR("Invalid param"); \
	}

using namespace smartcard_service_api;

EXTERN_API se_service_h se_service_create_instance(void *user_data,
	se_service_connected_cb callback)
{
	SEService *service;

	try
	{
		service = new SEService(user_data, (serviceConnected)callback);
	}
	catch (...)
	{
		service = NULL;
	}

	return (se_service_h)service;
}

EXTERN_API se_service_h se_service_create_instance_with_event_callback(
	void *user_data, se_service_connected_cb connected,
	se_service_event_cb event, se_sesrvice_error_cb error)
{
	SEService *service;

	try
	{
		service = new SEService(user_data, (serviceConnected)connected);
	}
	catch (...)
	{
		service = NULL;
	}

	return (se_service_h)service;
}

EXTERN_API se_service_h se_service_create_instance_sync(void *user_data,
	int *result)
{
	SEService *service;

	try
	{
		service = new SEService(user_data);
	}
	catch (ExceptionBase &e)
	{
		*result = e.getErrorCode();
		service = NULL;
	}
	catch (...)
	{
		*result = SCARD_ERROR_UNKNOWN;
		service = NULL;
	}

	return (se_service_h)service;
}

EXTERN_API int se_service_get_version(se_service_h handle, char **version_str)
{
	int ret = 0;

	if (version_str == NULL) {
		return SCARD_ERROR_ILLEGAL_PARAM;
	}

	SE_SERVICE_EXTERN_BEGIN;

	*version_str = g_strdup(service->getVersion());

	SE_SERVICE_EXTERN_END;

	return ret;
}

EXTERN_API int se_service_get_readers_count(se_service_h handle)
{
	int count = 0;

	SE_SERVICE_EXTERN_BEGIN;

	vector<ReaderHelper *> temp_readers;

	temp_readers = service->getReaders();
	count = temp_readers.size();

	SE_SERVICE_EXTERN_END;

	return count;
}

EXTERN_API int se_service_get_readers(se_service_h handle, int **readers, int *count)
{
	int result = 0;

	SE_SERVICE_EXTERN_BEGIN;

	vector<ReaderHelper *> temp_readers;
	size_t i;
	int temp = 0;

	temp_readers = service->getReaders();
	if(temp_readers.size() > 0)
	{
		*readers = (int *)calloc(temp_readers.size(), sizeof(int));

		if(*readers == NULL)
		{
			*count = 0;
			return SCARD_ERROR_NOT_ENOUGH_RESOURCE;
		}

		for (i = 0; i < temp_readers.size(); i++)
		{
			if (temp_readers[i]->isSecureElementPresent())
			{
				(*readers)[i] = (long)temp_readers[i];
				temp++;
			}
		}
		*count = temp;
	}
	else
	{
		*count = 0;
	}

	SE_SERVICE_EXTERN_END;

	return result;
}

EXTERN_API bool se_service_is_connected(se_service_h handle)
{
	bool result = false;

	SE_SERVICE_EXTERN_BEGIN;

	result = service->isConnected();

	SE_SERVICE_EXTERN_END;

	return result;
}

EXTERN_API void se_service_shutdown(se_service_h handle)
{
	SE_SERVICE_EXTERN_BEGIN;

	service->shutdownSync();

	SE_SERVICE_EXTERN_END;
}

EXTERN_API int se_service_destroy_instance(se_service_h handle)
{
	int result = 0;

	SE_SERVICE_EXTERN_BEGIN;

	delete service;

	SE_SERVICE_EXTERN_END;

	return result;
}
