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


#ifndef MESSAGE_H_
#define MESSAGE_H_

/* standard library header */

/* SLP library header */

/* local header */
#include "Serializable.h"

namespace smartcard_service_api
{
	class Message: public Serializable
	{
	private:
		char text[200];

	public:
		static const int MSG_REQUEST_READERS = 0x80;
		static const int MSG_REQUEST_SHUTDOWN = 0x81;
		static const int MSG_REQUEST_OPEN_SESSION = 0x82;
		static const int MSG_REQUEST_CLOSE_SESSION = 0x83;
		static const int MSG_REQUEST_OPEN_CHANNEL = 0x84;
		static const int MSG_REQUEST_CLOSE_CHANNEL = 0x85;
		static const int MSG_REQUEST_GET_ATR = 0x86;
		static const int MSG_REQUEST_TRANSMIT = 0x87;
		static const int MSG_REQUEST_GET_CHANNEL_COUNT = 0x88;

		static const int MSG_NOTIFY_SE_REMOVED = 0x90;
		static const int MSG_NOTIFY_SE_INSERTED = 0x91;

		static const int MSG_OPERATION_RELEASE_CLIENT = 0xC0;

		unsigned int message;
		unsigned int param1;
		unsigned int param2;
		ByteArray data;
		int error;
		void *caller;
		void *callback;
		void *userParam;

		Message();
		~Message();

		ByteArray serialize();
		void deserialize(unsigned char *buffer, unsigned int length);
		void deserialize(ByteArray buffer);

		const char *toString();
	};

} /* namespace smartcard_service_api */
#endif /* MESSAGE_H_ */
