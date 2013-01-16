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

/* SLP library header */

/* local header */
#include "Debug.h"
#include "ServiceInstance.h"
#include "ClientInstance.h"
#include "ServerResource.h"

namespace smartcard_service_api
{
	unsigned int ServiceInstance::openSession(Terminal *terminal, vector<ByteArray> &certHashes, void *caller)
	{
		unsigned int handle = IntegerHandle::assignHandle();

		ServerSession *session = new ServerSession((ServerReader *)0, certHashes, caller, terminal);

		mapSessions.insert(make_pair(handle, make_pair(session, terminal)));

		return handle;
	}

	ServerSession *ServiceInstance::getSession(unsigned int session)
	{
		ServerSession *result = NULL;
		map<unsigned int, pair<ServerSession *, Terminal *> >::iterator item;

		if ((item = mapSessions.find(session)) != mapSessions.end())
		{
			/*if (item->second.first == session)*/
				result = item->second.first;
		}

		return result;
	}

	Terminal *ServiceInstance::getTerminal(unsigned int session)
	{
		Terminal *result = NULL;
		map<unsigned int, pair<ServerSession *, Terminal *> >::iterator item;

		if ((item = mapSessions.find(session)) != mapSessions.end())
		{
			result = item->second.second;
		}

		return result;
	}

	void ServiceInstance::closeSession(unsigned int session)
	{
		map<unsigned int, pair<ServerSession *, Terminal *> >::iterator item;

		if ((item = mapSessions.find(session)) != mapSessions.end())
		{
			closeChannelsBySession(session);

			item->second.first->closeSync();

			mapSessions.erase(item);

			IntegerHandle::releaseHandle(session);
		}
	}

	void ServiceInstance::closeSessions()
	{
		map<unsigned int, pair<ServerSession *, Terminal *> >::iterator item;

		closeChannels();

		for (item = mapSessions.begin(); item != mapSessions.end(); item++)
		{
			item->second.first->closeSync();

			IntegerHandle::releaseHandle(item->first);
		}

		mapSessions.clear();
	}

	unsigned int ServiceInstance::openChannel(unsigned int session, int channelNum, ByteArray response)
	{
		Terminal *terminal = getTerminal(session);
		ServerChannel *channel = NULL;
		unsigned int handle = -1;

		/* create ServerChannel */
		channel = new ServerChannel((ServerSession *)session, (void *)parent->getPID(), channelNum, terminal);
		if (channel != NULL)
		{
			handle = IntegerHandle::assignHandle();
			mapChannels.insert(make_pair(handle, make_pair(session, channel)));

			if (response != ByteArray::EMPTY)
				channel->selectResponse = response;
		}
		else
		{
			SCARD_DEBUG_ERR("alloc failed");
		}

		return handle;
	}

	ServerChannel *ServiceInstance::getChannel(/*unsigned int session, */unsigned int channel)
	{
		ServerChannel *result = NULL;
		map<unsigned int, pair<unsigned int, ServerChannel *> >::iterator item;

		if ((item = mapChannels.find(channel)) != mapChannels.end())
		{
			/*if (item->second.first == session)*/
				result = item->second.second;
		}

		return result;
	}

	unsigned int ServiceInstance::getChannelCountBySession(unsigned int session)
	{
		unsigned int channelCount = 0;
		map<unsigned int, pair<unsigned int, ServerChannel *> >::iterator item;

		for (item = mapChannels.begin(); item != mapChannels.end(); item++)
		{
			if (item->second.first == session)
				channelCount++;
		}

		return channelCount;
	}

	void ServiceInstance::closeChannel(unsigned int channel)
	{
		map<unsigned int, pair<unsigned int, ServerChannel *> >::iterator item;

		if ((item = mapChannels.find(channel)) != mapChannels.end())
		{
			/* destroy ServerChannel */
			delete item->second.second;

			mapChannels.erase(item);

			IntegerHandle::releaseHandle(channel);
		}
	}

	void ServiceInstance::closeChannelsBySession(unsigned int session)
	{
		size_t i;
		vector<unsigned int> list;
		map<unsigned int, pair<unsigned int, ServerChannel *> >::iterator item;

		for (item = mapChannels.begin(); item != mapChannels.end(); item++)
		{
			if (item->second.first == session)
				list.push_back(item->second.first);
		}

		for (i = 0; i < list.size(); i++)
		{
			closeChannel(list[i]);
		}
	}

	void ServiceInstance::closeChannels()
	{
		map<unsigned int, pair<unsigned int, ServerChannel *> >::iterator item;

		for (item = mapChannels.begin(); item != mapChannels.end(); item++)
		{
			/* destroy ServerChannel */
			delete item->second.second;

			IntegerHandle::releaseHandle(item->first);
		}

		mapChannels.clear();
	}

} /* namespace smartcard_service_api */
