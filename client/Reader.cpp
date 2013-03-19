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
#include <stdio.h>
#include <string.h>
#include <unistd.h>

/* SLP library header */

/* local header */
#include "Debug.h"
#include "Message.h"
#include "ClientIPC.h"
#include "Reader.h"
#include "Session.h"

#ifndef EXTERN_API
#define EXTERN_API __attribute__((visibility("default")))
#endif

namespace smartcard_service_api
{
	Reader::Reader(void *context, const char *name, void *handle)
		: ReaderHelper()
	{
		unsigned int length = 0;

		SCARD_BEGIN();

		this->context = NULL;
		this->handle = NULL;

		if (context == NULL || name == NULL || strlen(name) == 0 || handle == NULL)
		{
			SCARD_DEBUG_ERR("invalid param");

			return;
		}

		this->handle = handle;
		this->context = context;

		length = strlen(name);
		length = (length < sizeof(this->name)) ? length : sizeof(this->name);
		memcpy(this->name, name, length);

		present = true;

		SCARD_END();
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
		throw(ErrorIO &, ErrorIllegalState &, ErrorIllegalParameter &, ErrorSecurity &)
	{
		openedSession = NULL;

		if (isSecureElementPresent() == true)
		{
			Message msg;
			int rv;

#ifdef CLIENT_IPC_THREAD
			/* request channel handle from server */
			msg.message = Message::MSG_REQUEST_OPEN_SESSION;
			msg.param1 = (unsigned long)handle;
			msg.error = (unsigned long)context; /* using error to context */
			msg.caller = (void *)this;
			msg.callback = (void *)this; /* if callback is class instance, it means synchronized call */

			syncLock();
			if (ClientIPC::getInstance().sendMessage(&msg) == true)
			{
				rv = waitTimedCondition(0);
				if (rv != 0)
				{
					SCARD_DEBUG_ERR("time over");
					this->error = SCARD_ERROR_OPERATION_TIMEOUT;
				}
			}
			else
			{
				SCARD_DEBUG_ERR("sendMessage failed");
				this->error = SCARD_ERROR_IPC_FAILED;
			}
			syncUnlock();

			if (this->error != SCARD_ERROR_OK)
			{
				ThrowError::throwError(this->error);
			}
#endif
		}
		else
		{
			SCARD_DEBUG_ERR("unavailable reader");
			throw ErrorIllegalState(SCARD_ERROR_UNAVAILABLE);
		}

		return (Session *)openedSession;
	}

	int Reader::openSession(openSessionCallback callback, void *userData)
	{
		int result;

		SCARD_BEGIN();

		if (isSecureElementPresent() == true)
		{
			Message msg;

			/* request channel handle from server */
			msg.message = Message::MSG_REQUEST_OPEN_SESSION;
			msg.param1 = (unsigned long)handle;
			msg.error = (unsigned long)context; /* using error to context */
			msg.caller = (void *)this;
			msg.callback = (void *)callback;
			msg.userParam = userData;

			if (ClientIPC::getInstance().sendMessage(&msg) == true)
			{
				result = SCARD_ERROR_OK;
			}
			else
			{
				SCARD_DEBUG_ERR("sendMessage failed");
				result = SCARD_ERROR_IPC_FAILED;
			}
		}
		else
		{
			SCARD_DEBUG_ERR("unavailable reader");
			result = SCARD_ERROR_ILLEGAL_STATE;
		}

		SCARD_END();

		return result;
	}

	bool Reader::dispatcherCallback(void *message)
	{
		Message *msg = (Message *)message;
		Reader *reader;
		bool result = false;

		SCARD_BEGIN();

		if (msg == NULL)
		{
			SCARD_DEBUG_ERR("message is null");
			return result;
		}

		reader = (Reader *)msg->caller;

		switch (msg->message)
		{
		case Message::MSG_REQUEST_OPEN_SESSION :
			{
				Session *session = NULL;

				SCARD_DEBUG("MSG_REQUEST_OPEN_SESSION");

				if (msg->param1 != 0)
				{
					/* create new instance of channel */
					session = new Session(reader->context, reader, (void *)msg->param1);
					if (session == NULL)
					{
						SCARD_DEBUG_ERR("Session creating instance failed");

						return session;
					}

					reader->sessions.push_back(session);
				}

				if (msg->isSynchronousCall() == true) /* synchronized call */
				{
					/* sync call */
					reader->syncLock();

					/* copy result */
					reader->error = msg->error;
					reader->openedSession = session;
					reader->signalCondition();

					reader->syncUnlock();
				}
				else if (msg->callback != NULL)
				{
					openSessionCallback cb = (openSessionCallback)msg->callback;

					/* async call */
					cb(session, msg->error, msg->userParam);
				}
			}
			break;

		default:
			SCARD_DEBUG("unknown [%s]", msg->toString());
			break;
		}

		SCARD_END();

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
		SCARD_DEBUG_ERR("Invalid param"); \
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

#ifdef CLIENT_IPC_THREAD
	READER_EXTERN_BEGIN;
	result = (session_h)reader->openSessionSync();
	READER_EXTERN_END;
#endif

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
