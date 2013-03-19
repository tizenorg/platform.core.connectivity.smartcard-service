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

/* SLP library header */

/* local header */
#include "Debug.h"
#include "Message.h"
#include "ClientIPC.h"
#include "ClientChannel.h"
#include "ReaderHelper.h"
#include "APDUHelper.h"

#ifndef EXTERN_API
#define EXTERN_API __attribute__((visibility("default")))
#endif

namespace smartcard_service_api
{
	ClientChannel::ClientChannel(void *context, Session *session,
		int channelNum, ByteArray selectResponse, void *handle)
		: Channel(session)
	{
		this->channelNum = -1;
		this->handle = NULL;
		this->context = NULL;

		if (handle == NULL)
		{
			SCARD_DEBUG_ERR("ClientIPC::getInstance() failed");

			return;
		}

		this->channelNum = channelNum;
		this->handle = handle;
		this->selectResponse = selectResponse;
		this->context = context;
	}

	ClientChannel::~ClientChannel()
	{
		closeSync();
	}

	void ClientChannel::closeSync() throw(ErrorIO &, ErrorIllegalState &)
	{
#ifdef CLIENT_IPC_THREAD
		if (isClosed() == false)
		{
			if (getSession()->getReader()->isSecureElementPresent() == true)
			{
				Message msg;
				int rv;

				/* send message to server */
				msg.message = Message::MSG_REQUEST_CLOSE_CHANNEL;
				msg.param1 = (unsigned long)handle;
				msg.error = (unsigned long)context; /* using error to context */
				msg.caller = (void *)this;
				msg.callback = (void *)this; /* if callback is class instance, it means synchronized call */

				syncLock();
				if (ClientIPC::getInstance().sendMessage(&msg) == true)
				{
					rv = waitTimedCondition(0);
					if (rv < 0)
					{
						SCARD_DEBUG_ERR("closeSync failed [%d]", rv);
						this->error = SCARD_ERROR_OPERATION_TIMEOUT;
					}
				}
				else
				{
					SCARD_DEBUG_ERR("sendMessage failed");
					this->error = SCARD_ERROR_IPC_FAILED;
				}
				syncUnlock();

				channelNum = -1;

				if (this->error != SCARD_ERROR_OK)
				{
					ThrowError::throwError(this->error);
				}
			}
			else
			{
				/* FIXME */
				SCARD_DEBUG("unavailable channel");
			}
		}
#endif
	}

	int ClientChannel::close(closeCallback callback, void *userParam)
	{
		int result = SCARD_ERROR_OK;

		if (isClosed() == false)
		{
			if (getSession()->getReader()->isSecureElementPresent() == true)
			{
				Message msg;
				channelNum = -1;

				/* send message to server */
				msg.message = Message::MSG_REQUEST_CLOSE_CHANNEL;
				msg.param1 = (unsigned long)handle;
				msg.error = (unsigned long)context; /* using error to context */
				msg.caller = (void *)this;
				msg.callback = (void *)callback;
				msg.userParam = userParam;

				if (ClientIPC::getInstance().sendMessage(&msg) == false)
				{
					result = SCARD_ERROR_IPC_FAILED;
				}
			}
			else
			{
				SCARD_DEBUG_ERR("unavailable channel");
				result = SCARD_ERROR_ILLEGAL_STATE;
			}
		}

		return result;
	}

	int ClientChannel::transmitSync(ByteArray command, ByteArray &result)
		throw(ErrorIO &, ErrorIllegalState &, ErrorIllegalParameter &, ErrorSecurity &)
	{
		int rv = SCARD_ERROR_OK;
		if (getSession()->getReader()->isSecureElementPresent() == true)
		{
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
			if (ClientIPC::getInstance().sendMessage(&msg) == true)
			{
				rv = waitTimedCondition(0);
				if (rv >= 0)
				{
					result = response;

					rv = SCARD_ERROR_OK;
				}
				else
				{
					SCARD_DEBUG_ERR("timeout");

					this->error = SCARD_ERROR_OPERATION_TIMEOUT;
				}
			}
			else
			{
				SCARD_DEBUG_ERR("sendMessage failed");
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
			SCARD_DEBUG_ERR("unavailable channel");
			throw ErrorIllegalState(SCARD_ERROR_UNAVAILABLE);
		}

		return rv;
	}

	int ClientChannel::transmit(ByteArray command, transmitCallback callback, void *userParam)
	{
		int result;

		if (getSession()->getReader()->isSecureElementPresent() == true)
		{
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

			if (ClientIPC::getInstance().sendMessage(&msg) == true)
			{
				result = SCARD_ERROR_OK;
			}
			else
			{
				result = SCARD_ERROR_IPC_FAILED;
			}
		}
		else
		{
			SCARD_DEBUG_ERR("unavailable channel");
			result = SCARD_ERROR_ILLEGAL_STATE;
		}

		return result;
	}

	bool ClientChannel::dispatcherCallback(void *message)
	{
		Message *msg = (Message *)message;
		ClientChannel *channel = NULL;
		bool result = false;

		if (msg == NULL)
		{
			SCARD_DEBUG_ERR("message is null");
			return result;
		}

		channel = (ClientChannel *)msg->caller;

		switch (msg->message)
		{
		case Message::MSG_REQUEST_TRANSMIT :
			{
				/* transmit result */
				SCARD_DEBUG("MSG_REQUEST_TRANSMIT");

				if (msg->error == 0 &&
					ResponseHelper::getStatus(msg->data) == 0)
				{
					/* store select response */
					if (msg->data.getAt(1) == APDUCommand::INS_SELECT_FILE)
						channel->setSelectResponse(msg->data);
				}

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
					cb(msg->data.getBuffer(), msg->data.getLength(), msg->error, msg->userParam);
				}
			}
			break;

		case Message::MSG_REQUEST_CLOSE_CHANNEL :
			{
				SCARD_DEBUG("MSG_REQUEST_CLOSE_CHANNEL");

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
					closeCallback cb = (closeCallback)msg->callback;

					/* async call */
					cb(msg->error, msg->userParam);
				}
			}
			break;

		default:
			SCARD_DEBUG("unknwon message : %s", msg->toString());
			break;
		}

		return result;
	}
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
		SCARD_DEBUG_ERR("Invalid param"); \
	}

using namespace smartcard_service_api;

EXTERN_API int channel_close(channel_h handle, channel_close_cb callback, void *userParam)
{
	int result = -1;

	CHANNEL_EXTERN_BEGIN;
	result = channel->close((closeCallback)callback, userParam);
	CHANNEL_EXTERN_END;

	return result;
}

EXTERN_API int channel_transmit(channel_h handle, unsigned char *command,
	unsigned int length, channel_transmit_cb callback, void *userParam)
{
	int result = -1;

	CHANNEL_EXTERN_BEGIN;
	ByteArray temp;

	temp.setBuffer(command, length);
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

	temp.setBuffer(command, cmd_len);

	try
	{
		result = channel->transmitSync(temp, resp);
		if (resp.getLength() > 0)
		{
			*resp_len = resp.getLength();
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
	result = channel->getSelectResponse().getLength();
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
	if (response.getLength() > 0)
	{
		memcpy(buffer, response.getBuffer(), MIN(length, response.getLength()));
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
