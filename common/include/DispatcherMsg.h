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


#ifndef DISPATCHERMSG_H_
#define DISPATCHERMSG_H_

/* standard library header */

/* SLP library header */

/* local header */
#include "Message.h"

namespace smartcard_service_api
{
	class DispatcherMsg: public Message
	{
	private:
		int peerSocket;

	public:
		DispatcherMsg():Message()
		{
			peerSocket = -1;
		}

		DispatcherMsg(Message *msg):Message()
		{
			peerSocket = -1;
			message = msg->message;
			param1 = msg->param1;
			param2 = msg->param2;
			error = msg->error;
			data = msg->data;
			caller = msg->caller;
			callback = msg->callback;
			userParam = msg->userParam;
		}

		DispatcherMsg(Message *msg, int socket):Message()
		{
			peerSocket = socket;
			message = msg->message;
			param1 = msg->param1;
			param2 = msg->param2;
			error = msg->error;
			data = msg->data;
			caller = msg->caller;
			callback = msg->callback;
			userParam = msg->userParam;
		}

		~DispatcherMsg() {}

		inline int getPeerSocket() { return peerSocket; }
		inline void setPeerSocket(int socket) { peerSocket = socket; }
	};

} /* namespace smartcard_service_api */
#endif /* DISPATCHERMSG_H_ */
