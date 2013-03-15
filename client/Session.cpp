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
#include "ClientIPC.h"

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
			SCARD_DEBUG_ERR("handle is null");

			return;
		}

		this->context = context;
		this->handle = handle;
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

	ByteArray Session::getATRSync() throw (ErrorIO &, ErrorIllegalState &)
	{
		ByteArray result;

		if (getReader()->isSecureElementPresent() == true)
		{
			if (atr.isEmpty() == true)
			{
				Message msg;
				int rv;

#ifdef CLIENT_IPC_THREAD
				/* request channel handle from server */
				msg.message = Message::MSG_REQUEST_GET_ATR;
				msg.param1 = (unsigned int)handle;
				msg.error = (unsigned int)context; /* using error to context */
				msg.caller = (void *)this;
				msg.callback = (void *)this; /* if callback is class instance, it means synchronized call */

				syncLock();
				if (ClientIPC::getInstance().sendMessage(&msg) == true)
				{
					rv = waitTimedCondition(0);
					if (rv != 0)
					{
						SCARD_DEBUG_ERR("time over");
					}
				}
				else
				{
					SCARD_DEBUG_ERR("sendMessage failed");
				}
				syncUnlock();
#endif
			}

			result = atr;
		}
		else
		{
			SCARD_DEBUG_ERR("unavailable session");
		}

		return result;
	}

	int Session::getATR(getATRCallback callback, void *userData)
	{
		int result = -1;

		if (getReader()->isSecureElementPresent() == true)
		{
			if (atr.isEmpty() == true)
			{
				Message msg;

				/* request channel handle from server */
				msg.message = Message::MSG_REQUEST_GET_ATR;
				msg.param1 = (unsigned int)handle;
				msg.error = (unsigned int)context; /* using error to context */
				msg.caller = (void *)this;
				msg.callback = (void *)callback;
				msg.userParam = userData;

				if (ClientIPC::getInstance().sendMessage(&msg) == true)
				{
					result = 0;
				}
			}
			else
			{
				/* TODO : invoke callback directly */
				callback(atr.getBuffer(), atr.getLength(), 0, userData);
			}
		}
		else
		{
			SCARD_DEBUG_ERR("unavailable session");
		}

		return result;
	}

	void Session::closeSync() throw (ErrorIO &, ErrorIllegalState &)
	{
		Message msg;
		int rv;

#ifdef CLIENT_IPC_THREAD
		if (isClosed() == false)
		{
			closed = true;
			closeChannels();

			/* request channel handle from server */
			msg.message = Message::MSG_REQUEST_CLOSE_SESSION;
			msg.param1 = (unsigned int)handle;
			msg.error = (unsigned int)context; /* using error to context */
			msg.caller = (void *)this;
			msg.callback = (void *)this; /* if callback is class instance, it means synchronized call */

			syncLock();
			if (ClientIPC::getInstance().sendMessage(&msg) == true)
			{
				rv = waitTimedCondition(0);

				if (rv != 0)
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

	int Session::close(closeSessionCallback callback, void *userData)
	{
		int result = -1;
		Message msg;

		if (isClosed() == false)
		{
			closed = true;
			closeChannels();

			/* request channel handle from server */
			msg.message = Message::MSG_REQUEST_CLOSE_SESSION;
			msg.param1 = (unsigned int)handle;
			msg.error = (unsigned int)context; /* using error to context */
			msg.caller = (void *)this;
			msg.callback = (void *)callback;
			msg.userParam = userData;

			if (ClientIPC::getInstance().sendMessage(&msg) == true)
			{
				result = 0;
			}
		}

		return result;
	}

	unsigned int Session::getChannelCountSync()
	{
		channelCount = -1;

		if (getReader()->isSecureElementPresent() == true)
		{
			Message msg;
			int rv;

#ifdef CLIENT_IPC_THREAD
			/* request channel handle from server */
			msg.message = Message::MSG_REQUEST_GET_CHANNEL_COUNT;
			msg.param1 = (unsigned int)handle;
			msg.error = (unsigned int)context; /* using error to context */
			msg.caller = (void *)this;
			msg.callback = (void *)this; /* if callback is class instance, it means synchronized call */

			channelCount = -1;

			syncLock();
			if (ClientIPC::getInstance().sendMessage(&msg) == true)
			{
				rv = waitTimedCondition(0);
				if (rv != 0)
				{
					SCARD_DEBUG_ERR("time over");
				}
			}
			else
			{
				SCARD_DEBUG_ERR("sendMessage failed");
			}
			syncUnlock();
#endif
		}
		else
		{
			SCARD_DEBUG_ERR("unavailable session");
		}

		return channelCount;
	}

	int Session::getChannelCount(getChannelCountCallback callback, void *userData)
	{
		int result = -1;

		if (getReader()->isSecureElementPresent() == true)
		{
			Message msg;

			msg.message = Message::MSG_REQUEST_GET_CHANNEL_COUNT;
			msg.param1 = (unsigned int)handle;
			msg.error = (unsigned int)context; /* using error to context */
			msg.caller = (void *)this;
			msg.callback = (void *)callback;
			msg.userParam = userData;

			if (ClientIPC::getInstance().sendMessage(&msg) == true)
			{
				result = 0;
			}
		}
		else
		{
			SCARD_DEBUG_ERR("unavailable session");
		}

		return result;
	}

	Channel *Session::openChannelSync(int id, ByteArray aid)
		throw (ErrorIO &, ErrorIllegalState &, ErrorIllegalParameter &, ErrorSecurity &)
	{
		openedChannel = NULL;

		if (getReader()->isSecureElementPresent() == true)
		{
			Message msg;
			int rv;

#ifdef CLIENT_IPC_THREAD
			/* request channel handle from server */
			msg.message = Message::MSG_REQUEST_OPEN_CHANNEL;
			msg.param1 = id;
			msg.param2 = (unsigned int)handle;
			msg.data = aid;
			msg.error = (unsigned int)context; /* using error to context */
			msg.caller = (void *)this;
			msg.callback = (void *)this; /* if callback is class instance, it means synchronized call */

			syncLock();
			if (ClientIPC::getInstance().sendMessage(&msg) == true)
			{
				rv = waitTimedCondition(0);
				if (rv != 0)
				{
					SCARD_DEBUG_ERR("time over");
				}
			}
			else
			{
				SCARD_DEBUG_ERR("sendMessage failed");
			}
			syncUnlock();
#endif
		}
		else
		{
			SCARD_DEBUG_ERR("unavailable session");
		}

		return (Channel *)openedChannel;
	}

	int Session::openChannel(int id, ByteArray aid, openChannelCallback callback, void *userData)
	{
		int result = -1;

		if (getReader()->isSecureElementPresent() == true)
		{
			Message msg;

			/* request channel handle from server */
			msg.message = Message::MSG_REQUEST_OPEN_CHANNEL;
			msg.param1 = id;
			msg.param2 = (unsigned int)handle;
			msg.data = aid;
			msg.error = (unsigned int)context; /* using error to context */
			msg.caller = (void *)this;
			msg.callback = (void *)callback;
			msg.userParam = userData;

			if (ClientIPC::getInstance().sendMessage(&msg) == true)
			{
				result = 0;
			}
		}
		else
		{
			SCARD_DEBUG_ERR("unavailable session");
		}

		return result;
	}

	Channel *Session::openBasicChannelSync(ByteArray aid)
		throw (ErrorIO &, ErrorIllegalState &, ErrorIllegalParameter &, ErrorSecurity &)
	{
		return openChannelSync(0, aid);
	}

	Channel *Session::openBasicChannelSync(unsigned char *aid, unsigned int length)
		throw (ErrorIO &, ErrorIllegalState &, ErrorIllegalParameter &, ErrorSecurity &)
	{
		return openBasicChannelSync(ByteArray(aid, length));
	}

	int Session::openBasicChannel(ByteArray aid, openChannelCallback callback, void *userData)
	{
		return openChannel(0, aid, callback, userData);
	}

	int Session::openBasicChannel(unsigned char *aid, unsigned int length,
		openChannelCallback callback, void *userData)
	{
		return openBasicChannel(ByteArray(aid, length), callback, userData);
	}

	Channel *Session::openLogicalChannelSync(ByteArray aid)
		throw (ErrorIO &, ErrorIllegalState &, ErrorIllegalParameter &, ErrorSecurity &)
	{
		return openChannelSync(1, aid);
	}

	Channel *Session::openLogicalChannelSync(unsigned char *aid, unsigned int length)
		throw (ErrorIO &, ErrorIllegalState &, ErrorIllegalParameter &, ErrorSecurity &)
	{
		return openLogicalChannelSync(ByteArray(aid, length));
	}

	int Session::openLogicalChannel(ByteArray aid, openChannelCallback callback, void *userData)
	{
		return openChannel(1, aid, callback, userData);
	}

	int Session::openLogicalChannel(unsigned char *aid, unsigned int length,
		openChannelCallback callback, void *userData)
	{
		return openLogicalChannel(ByteArray(aid, length), callback, userData);
	}

	bool Session::dispatcherCallback(void *message)
	{
		Message *msg = (Message *)message;
		Session *session = NULL;
		bool result = false;

		if (msg == NULL)
		{
			SCARD_DEBUG_ERR("message is null");
			return result;
		}

		session = (Session *)msg->caller;

		switch (msg->message)
		{
		case Message::MSG_REQUEST_OPEN_CHANNEL :
			{
				Channel *channel = NULL;

				SCARD_DEBUG("MSG_REQUEST_OPEN_CHANNEL");

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
						SCARD_DEBUG_ERR("alloc failed");

						msg->error = -1;
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
				SCARD_DEBUG("MSG_REQUEST_GET_ATR");

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
					cb(msg->data.getBuffer(), msg->data.getLength(), msg->error, msg->userParam);
				}
			}
			break;

		case Message::MSG_REQUEST_CLOSE_SESSION :
			{
				SCARD_DEBUG("MSG_REQUEST_CLOSE_SESSION");

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
				SCARD_DEBUG("MSG_REQUEST_GET_CHANNEL_COUNT");

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
			SCARD_DEBUG("unknown message : %s", msg->toString());
			break;
		}

		return result;
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
		SCARD_DEBUG_ERR("Invalid param"); \
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
		if (temp.getLength() > 0)
		{
			*length = temp.getLength();
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
