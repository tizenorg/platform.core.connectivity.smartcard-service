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

/* SLP library header */

/* local header */
#include "Debug.h"
#include "ClientIPC.h"
#include "ClientDispatcher.h"
#include "SEService.h"
#include "Reader.h"
#include "Message.h"

#ifndef EXTERN_API
#define EXTERN_API __attribute__((visibility("default")))
#endif

namespace smartcard_service_api
{
	SEService::SEService() :
		SEServiceHelper()
	{
		this->context = NULL;
		this->handler = NULL;
		this->listener = NULL;
		connected = false;
	}

	SEService::SEService(void *user_data, serviceConnected handler)
		throw(ErrorIO &, ErrorIllegalParameter &) :
		SEServiceHelper()
	{
		this->listener = NULL;
		connected = false;

		initialize(user_data, handler);
	}

	SEService::SEService(void *user_data, SEServiceListener *listener)
		throw(ErrorIO &, ErrorIllegalParameter &) :
		SEServiceHelper()
	{
		this->handler = NULL;
		connected = false;

		initialize(user_data, listener);
	}

	SEService::~SEService()
	{
		uint32_t i;

		try
		{
			shutdownSync();
		}
		catch(ExceptionBase &e)
		{
			_ERR("EXCEPTION : %s", e.what());
		}
		catch(...)
		{
			_ERR("EXCEPTION!!!");
		}

		for (i = 0; i < readers.size(); i++)
		{
			delete (Reader *)readers[i];
		}
		readers.clear();
	}

	SEService *SEService::createInstance(void *user_data, SEServiceListener *listener)
		throw(ErrorIO &, ErrorIllegalParameter &)
	{
		return new SEService(user_data, listener);
	}

	SEService *SEService::createInstance(void *user_data, serviceConnected handler)
		throw(ErrorIO &, ErrorIllegalParameter &)
	{
		return new SEService(user_data, handler);
	}

	void SEService::shutdown()
	{
		if (connected == true)
		{
			uint32_t i;

			for (i = 0; i < readers.size(); i++)
			{
				readers[i]->closeSessions();
			}

			Message msg;

			msg.message = Message::MSG_REQUEST_SHUTDOWN;
			msg.error = (unsigned long)this; /* using error to context */
			msg.caller = (void *)this;
			msg.callback = (void *)NULL;

			if (ClientIPC::getInstance().sendMessage(&msg) == false)
			{
				_ERR("time over");
			}
		}
	}

	void SEService::shutdownSync()
	{
#ifdef CLIENT_IPC_THREAD
		if (connected == true)
		{
			uint32_t i;

			for (i = 0; i < readers.size(); i++)
			{
				readers[i]->closeSessions();
			}

			/* send message to load se */
			Message msg;

			msg.message = Message::MSG_REQUEST_SHUTDOWN;
			msg.error = (unsigned long)this; /* using error to context */
			msg.caller = (void *)this;
			msg.callback = (void *)this; /* if callback is class instance, it means synchronized call */

			syncLock();
			if (ClientIPC::getInstance().sendMessage(&msg) == true)
			{
				int rv;

				rv = waitTimedCondition(0);

				if (rv == 0)
				{
					ClientDispatcher::getInstance().removeSEService(context);

					connected = false;
				}
				else
				{
					_ERR("time over");
				}
			}
			else
			{
				_ERR("sendMessage failed");
			}
			syncUnlock();
		}
#endif
	}

	bool SEService::_initialize() throw(ErrorIO &)
	{
		bool result = false;
		ClientIPC *clientIPC;
		ClientDispatcher *clientDispatcher;

		_BEGIN();

		/* initialize client */
		if (!g_thread_supported())
		{
			g_thread_init(NULL);
		}

		clientDispatcher = &ClientDispatcher::getInstance();
		clientIPC = &ClientIPC::getInstance();

		clientIPC->setDispatcher(clientDispatcher);

#ifndef CLIENT_IPC_THREAD
		if (clientDispatcher->runDispatcherThread() == false)
		{
			_ERR("clientDispatcher->runDispatcherThread() failed");

			return result;
		}
#endif

		if (clientIPC->createConnectSocket() == false)
		{
			_ERR("clientIPC->createConnectSocket() failed");

			return result;
		}

		clientDispatcher->addSEService(context, this);

		{
			/* send message to load se */
			Message msg;

			msg.message = Message::MSG_REQUEST_READERS;
			msg.error = getpid(); /* using error to pid */
			msg.caller = (void *)this;
			msg.userParam = context;

			result = clientIPC->sendMessage(&msg);
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

		return _initialize();
	}

	bool SEService::parseReaderInformation(unsigned int count, ByteArray data)
	{
		size_t i;
		unsigned int offset = 0;
		unsigned int len = 0;
		void *handle = NULL;
		Reader *reader = NULL;
		char name[100];

		for (i = 0; i < count && offset < data.getLength(); i++)
		{
			memset(name, 0, sizeof(name));

			memcpy(&len, data.getBuffer(offset), sizeof(len));
			offset += sizeof(len);

			memcpy(name, data.getBuffer(offset), len);
			offset += len;

			memcpy(&handle, data.getBuffer(offset), sizeof(handle));
			offset += sizeof(handle);

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

	bool SEService::dispatcherCallback(void *message)
	{
		Message *msg = (Message *)message;
		SEService *service = NULL;
		bool result = false;

		_BEGIN();

		if (msg == NULL)
		{
			_ERR("message is null");
			return result;
		}

		service = (SEService *)msg->caller;

		switch (msg->message)
		{
		case Message::MSG_REQUEST_READERS :
			_DBG("[MSG_REQUEST_READERS]");

			service->connected = true;

			/* parse message data */
			service->parseReaderInformation(msg->param1, msg->data);

			/* call callback function */
			if (service->listener != NULL)
			{
				service->listener->serviceConnected(service, service->context);
			}
			else if (service->handler != NULL)
			{
				service->handler(service, service->context);
			}
			break;

		case Message::MSG_REQUEST_SHUTDOWN :
			_DBG("[MSG_REQUEST_SHUTDOWN]");

			if (msg->isSynchronousCall() == true) /* synchronized call */
			{
				/* sync call */
				service->syncLock();

				/* copy result */
//				service->error = msg->error;
				service->signalCondition();
				service->syncUnlock();
			}
			else
			{
				/* Do nothing... */
			}
			break;

		case Message::MSG_NOTIFY_SE_INSERTED :
			{
				Reader *reader = NULL;

				_DBG("[MSG_NOTIFY_SE_INSERTED]");

				/* add readers */
				reader = new Reader(service->context,
					(char *)msg->data.getBuffer(), (void *)msg->param1);
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
						(char *)msg->data.getBuffer(), 1, service->context);
				}
				else
				{
					_DBG("listener is null");
				}
			}
			break;

		case Message::MSG_NOTIFY_SE_REMOVED :
			{
				size_t i;

				_DBG("[MSG_NOTIFY_SE_REMOVED]");

				for (i = 0; i < service->readers.size(); i++)
				{
					if (((Reader *)service->readers[i])->handle == (void *)msg->param1)
					{
						((Reader *)service->readers[i])->present = false;
						break;
					}
				}

				if (service->listener != NULL)
				{
					service->listener->eventHandler(service,
						(char *)msg->data.getBuffer(), 2, service->context);
				}
				else
				{
					_DBG("listener is null");
				}
			}
			break;

		case Message::MSG_OPERATION_RELEASE_CLIENT :
			_DBG("[MSG_OPERATION_RELEASE_CLIENT]");

			if (service->listener != NULL)
			{
				service->listener->errorHandler(service, msg->error, service->context);

				ClientDispatcher::getInstance().removeSEService(service->context);
				service->connected = false;
			}
			else
			{
				_ERR("service->listener is null");
			}
			break;

		default :
			_DBG("unknown message [%s]", msg->toString());
			break;
		}

		_END();

		return result;
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

EXTERN_API se_service_h se_service_create_instance(void *user_data, se_service_connected_cb callback)
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

EXTERN_API se_service_h se_service_create_instance_with_event_callback(void *user_data,
	se_service_connected_cb connected, se_service_event_cb event, se_sesrvice_error_cb error)
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

EXTERN_API bool se_service_get_readers(se_service_h handle, reader_h *readers, int *count)
{
	bool result = false;

	SE_SERVICE_EXTERN_BEGIN;

	vector<ReaderHelper *> temp_readers;
	size_t i;
	int temp = 0;

	temp_readers = service->getReaders();

	for (i = 0; i < temp_readers.size() && i < (size_t)*count; i++)
	{
		if (temp_readers[i]->isSecureElementPresent())
		{
			readers[i] = (reader_h)temp_readers[i];
			temp++;
		}
	}
	*count = temp;

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

	service->shutdown();

	SE_SERVICE_EXTERN_END;
}

EXTERN_API void se_service_destroy_instance(se_service_h handle)
{
	SE_SERVICE_EXTERN_BEGIN;

	delete service;

	SE_SERVICE_EXTERN_END;
}
