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


#ifndef SERVERCHANNEL_H_
#define SERVERCHANNEL_H_

/* standard library header */

/* SLP library header */

/* local header */
#include "Channel.h"
#include "Terminal.h"
#include "ServerSession.h"

namespace smartcard_service_api
{
	class ServerChannel: public Channel
	{
	private:
		Terminal *terminal;
		void *caller;
		bool privilege;

		ServerChannel(ServerSession *session, void *caller, int channelNum, Terminal *terminal);
		void setChannelNumber(int channelNum) { this->channelNum = channelNum; }
		void unsetPrivilegeMode() { this->privilege = false; }

	protected:
		void closeSync();
		int transmitSync(ByteArray command, ByteArray &result);

	public:
		~ServerChannel();

		int getChannelNumber() { return channelNum; }
		Terminal *getTerminal() { return terminal; }

		int close(closeCallback callback, void *userParam) { return -1; }
		int transmit(ByteArray command, transmitCallback callback, void *userParam) { return -1; };

		friend class ServerReader;
		friend class ServerSession;
		friend class ServiceInstance;
		friend class ServerResource;
	};

} /* namespace smartcard_service_api */
#endif /* SERVERCHANNEL_H_ */
