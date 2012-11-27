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
	SEService::SEService():SEServiceHelper()
	{
		pid = -1;
		this->context = NULL;
		this->handler = NULL;
		this->listener = NULL;
		connected = false;

		pid = getpid();
	}

	SEService::SEService(void *user_data, serviceConnected handler):SEServiceHelper()
	{
		pid = -1;
		this->context = NULL;
		this->handler = NULL;
		this->listener = NULL;
		connected = false;

		pid = getpid();

		initialize(user_data, handler);
	}

	SEService::SEService(void *user_data, SEServiceListener *listener):SEServiceHelper()
	{
		pid = -1;
		this->context = NULL;
		this->handler = NULL;
		this->listener = NULL;
		connected = false;

		pid = getpid();

		initialize(user_data, listener);
	}

	SEService::~SEService()
	{
		shutdownSync();
	}

	void SEService::shutdown()
	{
		uint32_t i;

		for (i = 0; i < readers.size(); i++)
		{
			readers[i]->closeSessions();
			delete (Reader *)readers[i];
		}

		readers.clear();

		Message msg;

		msg.message = Message::MSG_REQUEST_SHUTDOWN;
		msg.error = (unsigned int)this; /* using error to context */
		msg.caller = (void *)this;
		msg.callback = (void *)this; /* if callback is class instance, it means synchronized call */

		if (ClientIPC::getInstance().sendMessage(&msg) == false)
		{
			SCARD_DEBUG_ERR("time over");
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
				delete (Reader *)readers[i];
			}

			readers.clear();

			/* send message to load se */
			int rv;
			Message msg;

			msg.message = Message::MSG_REQUEST_SHUTDOWN;
			msg.error = (unsigned int)this; /* using error to context */
			msg.caller = (void *)this;
			msg.callback = (void *)this; /* if callback is class instance, it means synchronized call */

			syncLock();
			if (ClientIPC::getInstance().sendMessage(&msg) == true)
			{
				rv = waitTimedCondition(0);

				if (rv == 0)
				{
					ClientDispatcher::getInstance().removeSEService(context);

					connected = false;
				}
				else
				{
					SCARD_DEBUG_ERR("time over");
				}
			}
			else
			{
				SCARD_DEBUG_ERR("sendMessage failed");
			}
			syncUnlock();
		}
#endif
	}

	bool SEService::_initialize()
	{
		bool result = false;
		ClientIPC *clientIPC = NULL;
		ClientDispatcher *clientDispatcher = NULL;

		SCARD_BEGIN();

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
			SCARD_DEBUG_ERR("clientDispatcher->runDispatcherThread() failed");

			return result;
		}
#endif

		if (clientIPC->createConnectSocket() == false)
		{
			SCARD_DEBUG_ERR("clientIPC->createConnectSocket() failed");

			return result;
		}

		clientDispatcher->addSEService(context, this);

		{
			/* send message to load se */
			Message msg;

			msg.message = Message::MSG_REQUEST_READERS;
			msg.error = pid; /* using error to pid */
			msg.caller = (void *)this;
			msg.userParam = context;

			result = clientIPC->sendMessage(&msg);
		}

		SCARD_END();

		return result;
	}

	bool SEService::initialize(void *context, serviceConnected handler)
	{
		if (context == NULL)
		{
			SCARD_DEBUG_ERR("invalid param");
			return false;
		}

		this->context = context;
		this->handler = handler;

		return _initialize();
	}

	bool SEService::initialize(void *context, SEServiceListener *listener)
	{
		if (context == NULL)
		{
			SCARD_DEBUG_ERR("invalid param");
			return false;
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

			SCARD_DEBUG("Reader [%d] : name [%s], handle [%p]", i, name, handle);

			/* add readers */
			reader = new Reader(context, name, handle);
			if (reader == NULL)
			{
				SCARD_DEBUG_ERR("alloc failed");
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

		SCARD_BEGIN();

		if (msg == NULL)
		{
			SCARD_DEBUG_ERR("message is null");
			return result;
		}

		service = (SEService *)msg->caller;

		switch (msg->message)
		{
		case Message::MSG_REQUEST_READERS :
			SCARD_DEBUG("[MSG_REQUEST_READERS]");

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
			SCARD_DEBUG("[MSG_REQUEST_SHUTDOWN]");

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
//				openSessionCallback cb = (openSessionCallback)msg->callback;
//
//				/* async call */
//				cb(session, msg->error, msg->userParam);
			}
			break;

		case Message::MSG_NOTIFY_SE_INSERTED :
			{
				Reader *reader = NULL;

				SCARD_DEBUG("[MSG_NOTIFY_SE_INSERTED]");

				/* add readers */
				reader = new Reader(service->context, (char *)msg->data.getBuffer(), (void *)msg->param1);
				if (reader != NULL)
				{
					service->readers.push_back(reader);
				}
				else
				{
					SCARD_DEBUG_ERR("alloc failed");
				}

				if (service->listener != NULL)
				{
					service->listener->eventHandler(service, (char *)msg->data.getBuffer(), 1, service->context);
				}
				else
				{
					SCARD_DEBUG("listener is null");
				}
			}
			break;

		case Message::MSG_NOTIFY_SE_REMOVED :
			{
				size_t i;

				SCARD_DEBUG("[MSG_NOTIFY_SE_REMOVED]");

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
					service->listener->eventHandler(service, (char *)msg->data.getBuffer(), 2, service->context);
				}
				else
				{
					SCARD_DEBUG("listener is null");
				}
			}
			break;

		case Message::MSG_OPERATION_RELEASE_CLIENT :
			SCARD_DEBUG("[MSG_OPERATION_RELEASE_CLIENT]");

			if (service->listener != NULL)
			{
				service->listener->errorHandler(service, msg->error, service->context);

				ClientDispatcher::getInstance().removeSEService(service->context);
				service->connected = false;
			}
			else
			{
				SCARD_DEBUG_ERR("service->listener is null");
			}
			break;

		default:
			SCARD_DEBUG("unknown message [%s]", msg->toString());
			break;
		}

		SCARD_END();

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
		SCARD_DEBUG_ERR("Invalid param"); \
	}

using namespace smartcard_service_api;

EXTERN_API se_service_h se_service_create_instance(void *user_data, se_service_connected_cb callback)
{
	SEService *service = new SEService(user_data, (serviceConnected)callback);

	return (se_service_h)service;
}

EXTERN_API se_service_h se_service_create_instance_with_event_callback(void *user_data, se_service_connected_cb connected, se_service_event_cb event, se_sesrvice_error_cb error)
{
	SEService *service = new SEService(user_data, (serviceConnected)connected);

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
