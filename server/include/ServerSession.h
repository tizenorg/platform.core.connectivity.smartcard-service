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


#ifndef SERVERSESSION_H_
#define SERVERSESSION_H_

/* standard library header */
#include <vector>
#include <map>

/* SLP library header */

/* local header */
#include "ByteArray.h"
#include "Channel.h"
#include "Terminal.h"
#include "SessionHelper.h"

using namespace std;

namespace smartcard_service_api
{
	class ServerReader;

	class ServerSession : public SessionHelper
	{
	private:
		void *caller;
		Terminal *terminal;
		vector<ByteArray> certHashes;

		ServerSession(ServerReader *reader, vector<ByteArray> &certHashes, void *caller, Terminal *terminal);

		int getATR(getATRCallback callback, void *userData){ return -1; }
		int close(closeSessionCallback callback, void *userData){ return -1; }

		int openBasicChannel(ByteArray aid, openChannelCallback callback, void *userData){ return -1; }
		int openBasicChannel(unsigned char *aid, unsigned int length, openChannelCallback callback, void *userData){ return -1; }
		int openLogicalChannel(ByteArray aid, openChannelCallback callback, void *userData){ return -1; }
		int openLogicalChannel(unsigned char *aid, unsigned int length, openChannelCallback callback, void *userData){ return -1; }
	public:
		~ServerSession();

		ByteArray getATRSync()
			throw(ErrorIO &, ErrorIllegalState &);
		void closeSync()
			throw(ErrorIO &, ErrorIllegalState &);

		void closeChannels()
			throw(ErrorIO &, ErrorIllegalState &);

		Channel *openBasicChannelSync(ByteArray aid)
			throw(ErrorIO &, ErrorIllegalState &, ErrorIllegalParameter &, ErrorSecurity &);
		Channel *openBasicChannelSync(unsigned char *aid, unsigned int length)
			throw(ErrorIO &, ErrorIllegalState &, ErrorIllegalParameter &, ErrorSecurity &);
		Channel *openBasicChannelSync(ByteArray aid, void *caller)
			throw(ErrorIO &, ErrorIllegalState &, ErrorIllegalParameter &, ErrorSecurity &);
		Channel *openBasicChannelSync(unsigned char *aid, unsigned int length, void *caller)
			throw(ErrorIO &, ErrorIllegalState &, ErrorIllegalParameter &, ErrorSecurity &);

		Channel *openLogicalChannelSync(ByteArray aid)
			throw(ErrorIO &, ErrorIllegalState &, ErrorIllegalParameter &, ErrorSecurity &);
		Channel *openLogicalChannelSync(unsigned char *aid, unsigned int length)
			throw(ErrorIO &, ErrorIllegalState &, ErrorIllegalParameter &, ErrorSecurity &);
		Channel *openLogicalChannelSync(ByteArray aid, void *caller)
			throw(ErrorIO &, ErrorIllegalState &, ErrorIllegalParameter &, ErrorSecurity &);
		Channel *openLogicalChannelSync(unsigned char *aid, unsigned int length, void *caller)
			throw(ErrorIO &, ErrorIllegalState &, ErrorIllegalParameter &, ErrorSecurity &);

		friend class ServerReader;
		friend class ServerResource;
		friend class ServiceInstance;
	};

} /* namespace smartcard_service_api */
#endif /* SERVERSESSION_H_ */
