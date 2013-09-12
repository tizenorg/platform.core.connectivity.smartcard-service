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

#ifndef USE_GDBUS
/* standard library header */
#include <cstdio>
#include <cstring>
#include <sstream>

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

	const ByteArray Message::serialize() const
	{
		ByteArray result;
		unsigned int length = 0;
		unsigned int dataLength = 0;
		unsigned char *buffer = NULL;

		length = sizeof(message) + sizeof(param1) + sizeof(param2) + sizeof(error) + sizeof(caller) + sizeof(callback) + sizeof(userParam);
		if (data.size() > 0)
		{
			dataLength = data.size();
			length += sizeof(dataLength) + data.size();
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

			if (data.size() > 0)
			{
				memcpy(buffer + current, &dataLength, sizeof(dataLength));
				current += sizeof(dataLength);

				memcpy(buffer + current, data.getBuffer(), dataLength);
				current += data.size();
			}

			result.assign(buffer, length);

			delete []buffer;
		}
		else
		{
			_ERR("allocation failed");
		}

		return result;
	}

	void Message::deserialize(const ByteArray &buffer)
	{
		deserialize(buffer.getBuffer(), buffer.size());
	}

	void Message::deserialize(const unsigned char *buffer, unsigned int length)
	{
		unsigned int current = 0;
		unsigned int dataLength = 0;

//		_DBG("buffer [%p], length [%d]", buffer, length);

		memcpy(&message, buffer + current, sizeof(message));
		current += sizeof(message);

//		_DBG("message [%d]", message);

		memcpy(&param1, buffer + current, sizeof(param1));
		current += sizeof(param1);

//		_DBG("param1 [%d]", param1);

		memcpy(&param2, buffer + current, sizeof(param2));
		current += sizeof(param2);

//		_DBG("param2 [%d]", param2);

		memcpy(&error, buffer + current, sizeof(error));
		current += sizeof(error);

		memcpy(&caller, buffer + current, sizeof(caller));
		current += sizeof(caller);

		memcpy(&callback, buffer + current, sizeof(callback));
		current += sizeof(callback);

		memcpy(&userParam, buffer + current, sizeof(userParam));
		current += sizeof(userParam);

//		_DBG("userContext [%p]", userContext);

		if (current + sizeof(dataLength) < length)
		{
			memcpy(&dataLength, buffer + current, sizeof(dataLength));
			current += sizeof(dataLength);

//			_DBG("dataLength [%d]", dataLength);

			data.assign(buffer + current, dataLength);
			current += dataLength;
		}
	}

	const string Message::toString() const
	{
		stringstream ss;
		const char *msg = NULL;

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

		ss << "Message [" << msg << ", " << message << "], param1 [" << param1 << "], param2 [" << param2 << "], error [" << error << "], caller [" << "], callback [" << callback << "], userParam [" << userParam << "], data length [" << data.size() << "]";

		return ss.str();
	}

} /* namespace smartcard_service_api */
#endif
