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
		const vector<ByteArray> &certHashes,
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

	const ByteArray ServerSession::getATRSync()
		throw(ErrorIO &, ErrorIllegalState &)
	{
		/* call get atr to terminal */
		if (atr.isEmpty()) {
			if (terminal != NULL) {
				if (terminal->open() == true) {
					int error = terminal->getATRSync(atr);

					if (error < SCARD_ERROR_OK) {
						_ERR("getATRSync failed, [%d]", error);
					}

					terminal->close();
				} else {
					_ERR("terminal->open failed");
				}
			} else {
				_ERR("terminal is null.");
			}
		}

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

	Channel *ServerSession::openBasicChannelSync(const ByteArray &aid)
		throw(ErrorIO &, ErrorIllegalState &, ErrorIllegalParameter &, ErrorSecurity &)
	{
		return openBasicChannelSync(aid, (void *)NULL);
	}

	Channel *ServerSession::openBasicChannelSync(const ByteArray &aid, unsigned char P2)
		throw(ErrorIO &, ErrorIllegalState &, ErrorIllegalParameter &, ErrorSecurity &)
	{
		return openBasicChannelSync(aid, (void *)NULL);
	}

	Channel *ServerSession::openBasicChannelSync(const ByteArray &aid, void *caller)
		throw(ErrorIO &, ErrorIllegalState &, ErrorIllegalParameter &, ErrorSecurity &)
	{
		ServerChannel *channel = NULL;
#if 0
		AccessControlList *acList = NULL;
		ByteArray command, result;
		int channelID = 0;
		int rv = 0;

		_BEGIN();

		acList = ((ServerReader *)reader)->getAccessControlList();
		if (acList == NULL)
		{
			_ERR("acList is null");

			return channel;
		}

		if (acList->isAuthorizedAccess(aid, certHashes) == false)
		{
			_ERR("unauthorized access, aid : %s", aid.toString().c_str());

			return channel;
		}

		/* select aid */
		command = APDUHelper::generateAPDU(APDUHelper::COMMAND_SELECT_BY_DF_NAME, channelID, aid);
		rv = terminal->transmitSync(command, result);
		if (rv == 0 && result.size() >= 2)
		{
			ResponseHelper resp(result);

			if (resp.getStatus() == 0)
			{
				channel = new ServerChannel(this, caller, channelID, terminal);
				if (channel != NULL)
				{
					channel->selectResponse = result;

					channels.push_back(channel);
				}
				else
				{
					_ERR("alloc failed");
				}
			}
			else
			{
				_ERR("status word [ %02X %02X ]", resp.getSW1(), resp.getSW2());
			}
		}
		else
		{
			_ERR("select apdu is failed, rv [%d], length [%d]", rv, result.size());
		}
#endif
		return channel;
	}

	Channel *ServerSession::openBasicChannelSync(const unsigned char *aid, unsigned int length)
		throw(ErrorIO &, ErrorIllegalState &, ErrorIllegalParameter &, ErrorSecurity &)
	{
		unsigned char P2 = 0x00;
		ByteArray temp(aid, length);

		return openBasicChannelSync(temp, P2);
	}

	Channel *ServerSession::openBasicChannelSync(const unsigned char *aid, unsigned int length, unsigned char P2)
		throw(ErrorIO &, ErrorIllegalState &, ErrorIllegalParameter &, ErrorSecurity &)
	{
		ByteArray temp(aid, length);

		return openBasicChannelSync(temp, P2);
	}

	Channel *ServerSession::openBasicChannelSync(const unsigned char *aid, unsigned int length, void *caller)
		throw(ErrorIO &, ErrorIllegalState &, ErrorIllegalParameter &, ErrorSecurity &)
	{
		ByteArray temp(aid, length);

		return openBasicChannelSync(temp, caller);
	}

	Channel *ServerSession::openLogicalChannelSync(const ByteArray &aid)
		throw(ErrorIO &, ErrorIllegalState &, ErrorIllegalParameter &, ErrorSecurity &)
	{
		void* caller = NULL;
		return openLogicalChannelSync(aid, caller);
	}

	Channel *ServerSession::openLogicalChannelSync(const ByteArray &aid, unsigned char P2)
		throw(ErrorIO &, ErrorIllegalState &, ErrorIllegalParameter &, ErrorSecurity &)
	{
		void* caller = NULL;
		return openLogicalChannelSync(aid, caller);
	}

	Channel *ServerSession::openLogicalChannelSync(const ByteArray &aid, void *caller)
		throw(ErrorIO &, ErrorIllegalState &, ErrorIllegalParameter &, ErrorSecurity &)
	{
		ServerChannel *channel = NULL;
#if 0
		AccessControlList *acList = NULL;
		ByteArray command, result;
		int channelID = 1;
		int rv;

		acList = ((ServerReader *)reader)->getAccessControlList();
		if (acList == NULL)
		{
			_ERR("unauthorized access, aid %s, hash %s");

			return channel;
		}

		if (acList->isAuthorizedAccess(aid, certHashes) == false)
		{
			_ERR("unauthorized access, aid : %s", aid.toString().c_str());

			return channel;
		}

		/* open channel */
		command = APDUHelper::generateAPDU(APDUHelper::COMMAND_OPEN_LOGICAL_CHANNEL, 0, ByteArray::EMPTY);
		rv = terminal->transmitSync(command, result);

		if (rv == 0 && result.size() >= 2)
		{
			ResponseHelper resp(result);

			if (resp.getStatus() == 0)
			{
				channelID = resp.getDataField()[0];
			}
			else
			{
				_ERR("status word [ %02X %02X ]", resp.getSW1(), resp.getSW2());

				return channel;
			}
		}
		else
		{
			_ERR("select apdu is failed, rv [%d], length [%d]", rv, result.size());

			return channel;
		}

		/* select aid */
		command = APDUHelper::generateAPDU(APDUHelper::COMMAND_SELECT_BY_DF_NAME, channelID, aid);
		rv = terminal->transmitSync(command, result);

		if (rv == 0 && result.size() >= 2)
		{
			ResponseHelper resp(result);

			if (resp.getStatus() == 0)
			{
				channel = new ServerChannel(this, caller, channelID, terminal);
				if (channel == NULL)
				{
					_ERR("alloc failed");

					return NULL;
				}

				channel->selectResponse = result;

				channels.push_back(channel);
			}
			else
			{
				_ERR("status word [ %02X %02X ]", resp.getSW1(), resp.getSW2());
			}
		}
		else
		{
			_ERR("select apdu is failed, rv [%d], length [%d]", rv, result.size());
		}
#endif
		return channel;
	}

	Channel *ServerSession::openLogicalChannelSync(const unsigned char *aid, unsigned int length)
		throw(ErrorIO &, ErrorIllegalState &, ErrorIllegalParameter &, ErrorSecurity &)
	{
		unsigned char P2 = 0x00;
		ByteArray temp(aid, length);

		return openLogicalChannelSync(temp, P2);
	}

	Channel *ServerSession::openLogicalChannelSync(const unsigned char *aid, unsigned int length, unsigned char P2)
		throw(ErrorIO &, ErrorIllegalState &, ErrorIllegalParameter &, ErrorSecurity &)
	{
		ByteArray temp(aid, length);

		return openLogicalChannelSync(temp, P2);
	}

	Channel *ServerSession::openLogicalChannelSync(const unsigned char *aid, unsigned int length, void *caller)
		throw(ErrorIO &, ErrorIllegalState &, ErrorIllegalParameter &, ErrorSecurity &)
	{
		ByteArray temp(aid, length);

		return openLogicalChannelSync(temp, caller);
	}

} /* namespace smartcard_service_api */
