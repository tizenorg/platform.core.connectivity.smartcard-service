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

/* SLP library header */

/* local header */
#include "Debug.h"
#include "Message.h"

namespace smartcard_service_api
{
	Message::Message()
	{
		message = 0;
		param1 = 0;
		param2 = 0;
		error = 0;
		caller = NULL;
		callback = NULL;
		userParam = NULL;
	}

	Message::~Message()
	{
	}

	ByteArray Message::serialize()
	{
		ByteArray result;
		unsigned int length = 0;
		unsigned int dataLength = 0;
		unsigned char *buffer = NULL;

		length = sizeof(message) + sizeof(param1) + sizeof(param2) + sizeof(error) + sizeof(caller) + sizeof(callback) + sizeof(userParam);
		if (data.getLength() > 0)
		{
			dataLength = data.getLength();
			length += sizeof(dataLength) + data.getLength();
		}

		buffer = new unsigned char[length];
		if (buffer != NULL)
		{
			unsigned int current = 0;

			memset(buffer, 0, length);

			memcpy(buffer + current, &message, sizeof(message));
			current += sizeof(message);

			memcpy(buffer + current, &param1, sizeof(param1));
			current += sizeof(param1);

			memcpy(buffer + current, &param2, sizeof(param2));
			current += sizeof(param2);

			memcpy(buffer + current, &error, sizeof(error));
			current += sizeof(error);

			memcpy(buffer + current, &caller, sizeof(caller));
			current += sizeof(caller);

			memcpy(buffer + current, &callback, sizeof(callback));
			current += sizeof(callback);

			memcpy(buffer + current, &userParam, sizeof(userParam));
			current += sizeof(userParam);

			if (data.getLength() > 0)
			{
				memcpy(buffer + current, &dataLength, sizeof(dataLength));
				current += sizeof(dataLength);

				memcpy(buffer + current, data.getBuffer(), dataLength);
				current += data.getLength();
			}

			result.setBuffer(buffer, length);

			delete []buffer;
		}
		else
		{
			SCARD_DEBUG_ERR("allocation failed");
		}

		return result;
	}

	void Message::deserialize(ByteArray buffer)
	{
		deserialize(buffer.getBuffer(), buffer.getLength());
	}

	void Message::deserialize(unsigned char *buffer, unsigned int length)
	{
		unsigned int current = 0;
		unsigned int dataLength = 0;

//		SCARD_DEBUG("buffer [%p], length [%d]", buffer, length);

		memcpy(&message, buffer + current, sizeof(message));
		current += sizeof(message);

//		SCARD_DEBUG("message [%d]", message);

		memcpy(&param1, buffer + current, sizeof(param1));
		current += sizeof(param1);

//		SCARD_DEBUG("param1 [%d]", param1);

		memcpy(&param2, buffer + current, sizeof(param2));
		current += sizeof(param2);

//		SCARD_DEBUG("param2 [%d]", param2);

		memcpy(&error, buffer + current, sizeof(error));
		current += sizeof(error);

		memcpy(&caller, buffer + current, sizeof(caller));
		current += sizeof(caller);

		memcpy(&callback, buffer + current, sizeof(callback));
		current += sizeof(callback);

		memcpy(&userParam, buffer + current, sizeof(userParam));
		current += sizeof(userParam);

//		SCARD_DEBUG("userContext [%p]", userContext);

		if (current + sizeof(dataLength) < length)
		{
			memcpy(&dataLength, buffer + current, sizeof(dataLength));
			current += sizeof(dataLength);

//			SCARD_DEBUG("dataLength [%d]", dataLength);

			data.setBuffer(buffer + current, dataLength);
			current += dataLength;
		}
	}

	const char *Message::toString()
	{
		const char *msg = NULL;

		memset(&text, 0, sizeof(text));

		switch (message)
		{
		case MSG_REQUEST_READERS :
			msg = "MSG_REQUEST_READERS";
			break;

//		case MSG_REQUEST_READER_NAME :
//			msg = "MSG_REQUEST_READER_NAME";
//			break;
//
		case MSG_REQUEST_OPEN_SESSION :
			msg = "MSG_REQUEST_OPEN_SESSION";
			break;

		case MSG_REQUEST_CLOSE_SESSION :
			msg = "MSG_REQUEST_CLOSE_CHANNEL";
			break;

		case MSG_REQUEST_OPEN_CHANNEL :
			msg = "MSG_REQUEST_OPEN_CHANNEL";
			break;

		case MSG_REQUEST_CLOSE_CHANNEL :
			msg = "MSG_REQUEST_CLOSE_CHANNEL";
			break;

		case MSG_REQUEST_GET_ATR :
			msg = "MSG_REQUEST_GET_ATR";
			break;

		case MSG_REQUEST_TRANSMIT :
			msg = "MSG_REQUEST_TRANSMIT";
			break;

		case MSG_REQUEST_GET_CHANNEL_COUNT :
			msg = "MSG_REQUEST_GET_CHANNEL_COUNT";
			break;

		default :
			msg = "Unknown";
			break;
		}

		snprintf(text, sizeof(text), "Message [%s, %d], param1 [%d], param2 [%d], error [%d], caller [%p], callback [%p], userParam [%p], data length [%d]", msg, message, param1, param2, error, caller, callback, userParam, data.getLength());

		return (const char *)text;
	}

} /* namespace smartcard_service_api */
