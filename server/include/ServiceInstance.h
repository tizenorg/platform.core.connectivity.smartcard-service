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

#ifndef SERVICEINSTANCE_H_
#define SERVICEINSTANCE_H_

/* standard library header */
#include <map>

/* SLP library header */

/* local header */
#include "Terminal.h"
#include "ServerChannel.h"
#include "ServerSession.h"

namespace smartcard_service_api
{
	class ClientInstance;

	class ServiceInstance
	{
	private:
		unsigned int handle;
		ClientInstance *parent;
		map<unsigned int, pair<ServerSession *, Terminal *> > mapSessions; /* session unique id <-> terminal instance map */
		map<unsigned int, pair<unsigned int, ServerChannel *> > mapChannels; /* channel unique id <-> (session unique id, channel instance) map */

	public:
		ServiceInstance(ClientInstance *parent);
		~ServiceInstance();

		inline unsigned int getHandle() { return handle; }

		inline bool operator ==(const unsigned int &handle) const { return (this->handle == handle); }
		inline bool isVaildSessionHandle(unsigned int handle) { return (mapSessions.find(handle) != mapSessions.end()); }
		inline bool isVaildChannelHandle(unsigned int handle) { return (mapChannels.find(handle) != mapChannels.end()); }
		inline ClientInstance *getParent() { return parent; }

		unsigned int openSession(Terminal *terminal, vector<ByteArray> &certHashes, void *caller);
		ServerSession *getSession(unsigned int session);
		void closeSession(unsigned int session);
		void closeSessions();

		Terminal *getTerminal(unsigned int session);

		unsigned int openChannel(unsigned int session, int channelNum, ByteArray response = ByteArray::EMPTY);
		ServerChannel *getChannel(/*unsigned int session, */unsigned int channel);
		unsigned int getChannelCountBySession(unsigned int session);

		void closeChannel(unsigned int channel);
		void closeChannelsBySession(unsigned int session);
		void closeChannels();
	};

} /* namespace smartcard_service_api */
#endif /* SERVICEINSTANCE_H_ */
