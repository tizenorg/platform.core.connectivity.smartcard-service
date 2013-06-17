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
#include <dlfcn.h>

/* SLP library header */

/* local header */
#include "Debug.h"
#include "ServerSession.h"
#include "ServerReader.h"
#include "ServerChannel.h"
#include "APDUHelper.h"
#include "GPACE.h"

namespace smartcard_service_api
{
	ServerSession::ServerSession(ServerReader *reader,
		vector<ByteArray> &certHashes,
		void *caller, Terminal *terminal) : SessionHelper(reader)
	{
		this->terminal = NULL;

		if (terminal == NULL)
		{
			_ERR("invalid param");

			return;
		}

		this->terminal = terminal;
		this->certHashes = certHashes;
	}

	ServerSession::~ServerSession()
	{
		if (isClosed() == false)
			closeSync();
	}

	ByteArray ServerSession::getATRSync()
		throw(ErrorIO &, ErrorIllegalState &)
	{
		/* call get atr to terminal */
		return atr;
	}

	void ServerSession::closeSync()
		throw(ErrorIO &, ErrorIllegalState &)
	{
		if (isClosed() == false)
		{
			closed = true;
			closeChannels();
		}
	}

	void ServerSession::closeChannels()
		throw(ErrorIO &, ErrorIllegalState &)
	{
		size_t i;

		for (i = 0; i < channels.size(); i++)
		{
			if (channels[i] != NULL)
				channels[i]->closeSync();
		}

		channels.clear();
	}

	Channel *ServerSession::openBasicChannelSync(ByteArray &aid)
		throw(ErrorIO &, ErrorIllegalState &, ErrorIllegalParameter &, ErrorSecurity &)
	{
		return openBasicChannelSync(aid, NULL);
	}

	Channel *ServerSession::openBasicChannelSync(ByteArray &aid, void *caller)
		throw(ErrorIO &, ErrorIllegalState &, ErrorIllegalParameter &, ErrorSecurity &)
	{
		ServerChannel *channel = NULL;
		return channel;
	}

	Channel *ServerSession::openBasicChannelSync(unsigned char *aid, unsigned int length)
		throw(ErrorIO &, ErrorIllegalState &, ErrorIllegalParameter &, ErrorSecurity &)
	{
		ByteArray temp(aid, length);

		return openBasicChannelSync(temp);
	}

	Channel *ServerSession::openBasicChannelSync(unsigned char *aid, unsigned int length, void *caller)
		throw(ErrorIO &, ErrorIllegalState &, ErrorIllegalParameter &, ErrorSecurity &)
	{
		ByteArray temp(aid, length);

		return openBasicChannelSync(temp, caller);
	}

	Channel *ServerSession::openLogicalChannelSync(ByteArray &aid)
		throw(ErrorIO &, ErrorIllegalState &, ErrorIllegalParameter &, ErrorSecurity &)
	{
		return openLogicalChannelSync(aid, NULL);
	}

	Channel *ServerSession::openLogicalChannelSync(ByteArray &aid, void *caller)
		throw(ErrorIO &, ErrorIllegalState &, ErrorIllegalParameter &, ErrorSecurity &)
	{
		ServerChannel *channel = NULL;
		return channel;
	}

	Channel *ServerSession::openLogicalChannelSync(unsigned char *aid, unsigned int length)
		throw(ErrorIO &, ErrorIllegalState &, ErrorIllegalParameter &, ErrorSecurity &)
	{
		ByteArray temp(aid, length);

		return openLogicalChannelSync(temp, NULL);
	}

	Channel *ServerSession::openLogicalChannelSync(unsigned char *aid, unsigned int length, void *caller)
		throw(ErrorIO &, ErrorIllegalState &, ErrorIllegalParameter &, ErrorSecurity &)
	{
		ByteArray temp(aid, length);

		return openLogicalChannelSync(temp, caller);
	}

} /* namespace smartcard_service_api */
