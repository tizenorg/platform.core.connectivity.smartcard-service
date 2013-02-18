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
#include "GPSEACL.h"

namespace smartcard_service_api
{
	ServerSession::ServerSession(ServerReader *reader, vector<ByteArray> &certHashes, void *caller, Terminal *terminal):SessionHelper(reader)
	{
		this->caller = NULL;
		this->terminal = NULL;

		if (caller == NULL || terminal == NULL)
		{
			SCARD_DEBUG_ERR("invalid param");

			return;
		}

		this->caller = caller;
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

	Channel *ServerSession::openBasicChannelSync(ByteArray aid)
		throw(ErrorIO &, ErrorIllegalState &, ErrorIllegalParameter &, ErrorSecurity &)
	{
		return openBasicChannelSync(aid, NULL);
	}

	Channel *ServerSession::openBasicChannelSync(ByteArray aid, void *caller)
		throw(ErrorIO &, ErrorIllegalState &, ErrorIllegalParameter &, ErrorSecurity &)
	{
		ServerChannel *channel = NULL;
#if 0
		AccessControlList *acList = NULL;
		ByteArray command, result;
		int channelID = 0;
		int rv = 0;

		SCARD_BEGIN();

		acList = ((ServerReader *)reader)->getAccessControlList();
		if (acList == NULL)
		{
			SCARD_DEBUG_ERR("acList is null");

			return channel;
		}

		if (acList->isAuthorizedAccess(aid, certHashes) == false)
		{
			SCARD_DEBUG_ERR("unauthorized access, aid : %s", aid.toString());

			return channel;
		}

		/* select aid */
		command = APDUHelper::generateAPDU(APDUHelper::COMMAND_SELECT_BY_DF_NAME, channelID, aid);
		rv = terminal->transmitSync(command, result);
		if (rv == 0 && result.getLength() >= 2)
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
					SCARD_DEBUG_ERR("alloc failed");
				}
			}
			else
			{
				SCARD_DEBUG_ERR("status word [%d][ %02X %02X ]", resp.getStatus(), resp.getSW1(), resp.getSW2());
			}
		}
		else
		{
			SCARD_DEBUG_ERR("select apdu is failed, rv [%d], length [%d]", rv, result.getLength());
		}
#endif
		return channel;
	}

	Channel *ServerSession::openBasicChannelSync(unsigned char *aid, unsigned int length)
		throw(ErrorIO &, ErrorIllegalState &, ErrorIllegalParameter &, ErrorSecurity &)
	{
		return openBasicChannelSync(ByteArray(aid, length));
	}

	Channel *ServerSession::openBasicChannelSync(unsigned char *aid, unsigned int length, void *caller)
		throw(ErrorIO &, ErrorIllegalState &, ErrorIllegalParameter &, ErrorSecurity &)
	{
		return openBasicChannelSync(ByteArray(aid, length), caller);
	}

	Channel *ServerSession::openLogicalChannelSync(ByteArray aid)
		throw(ErrorIO &, ErrorIllegalState &, ErrorIllegalParameter &, ErrorSecurity &)
	{
		return openLogicalChannelSync(aid, NULL);
	}

	Channel *ServerSession::openLogicalChannelSync(ByteArray aid, void *caller)
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
			SCARD_DEBUG_ERR("unauthorized access, aid %s, hash %s");

			return channel;
		}

		if (acList->isAuthorizedAccess(aid, certHashes) == false)
		{
			SCARD_DEBUG_ERR("unauthorized access, aid : %s", aid.toString());

			return channel;
		}

		/* open channel */
		command = APDUHelper::generateAPDU(APDUHelper::COMMAND_OPEN_LOGICAL_CHANNEL, 0, ByteArray::EMPTY);
		rv = terminal->transmitSync(command, result);

		if (rv == 0 && result.getLength() >= 2)
		{
			ResponseHelper resp(result);

			if (resp.getStatus() == 0)
			{
				channelID = resp.getDataField()[0];
			}
			else
			{
				SCARD_DEBUG_ERR("status word [%d][ %02X %02X ]", resp.getStatus(), resp.getSW1(), resp.getSW2());

				return channel;
			}
		}
		else
		{
			SCARD_DEBUG_ERR("select apdu is failed, rv [%d], length [%d]", rv, result.getLength());

			return channel;
		}

		/* select aid */
		command = APDUHelper::generateAPDU(APDUHelper::COMMAND_SELECT_BY_DF_NAME, channelID, aid);
		rv = terminal->transmitSync(command, result);

		if (rv == 0 && result.getLength() >= 2)
		{
			ResponseHelper resp(result);

			if (resp.getStatus() == 0)
			{
				channel = new ServerChannel(this, caller, channelID, terminal);
				if (channel == NULL)
				{
					SCARD_DEBUG_ERR("alloc failed");

					return NULL;
				}

				channel->selectResponse = result;

				channels.push_back(channel);
			}
			else
			{
				SCARD_DEBUG_ERR("status word [%d][ %02X %02X ]", resp.getStatus(), resp.getSW1(), resp.getSW2());
			}
		}
		else
		{
			SCARD_DEBUG_ERR("select apdu is failed, rv [%d], length [%d]", rv, result.getLength());
		}
#endif
		return channel;
	}

	Channel *ServerSession::openLogicalChannelSync(unsigned char *aid, unsigned int length)
		throw(ErrorIO &, ErrorIllegalState &, ErrorIllegalParameter &, ErrorSecurity &)
	{
		return openLogicalChannelSync(ByteArray(aid, length), NULL);
	}

	Channel *ServerSession::openLogicalChannelSync(unsigned char *aid, unsigned int length, void *caller)
		throw(ErrorIO &, ErrorIllegalState &, ErrorIllegalParameter &, ErrorSecurity &)
	{
		return openLogicalChannelSync(ByteArray(aid, length), caller);
	}

} /* namespace smartcard_service_api */
