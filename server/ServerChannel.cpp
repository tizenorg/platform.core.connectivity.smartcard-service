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
#include "ServerChannel.h"
#include "APDUHelper.h"

namespace smartcard_service_api
{
	ServerChannel::ServerChannel(ServerSession *session, void *caller,
		int channelNum, Terminal *terminal)
		: Channel(session)
	{
		this->terminal = terminal;
		this->caller = caller;
		this->channelNum = channelNum;
		this->privilege = true;
	}

	ServerChannel::~ServerChannel()
	{
		if (isClosed() == false)
		{
			closeSync();
		}
	}

	void ServerChannel::closeSync()
		throw(ErrorIO &, ErrorIllegalState &)
	{
		ByteArray command, result;
		APDUHelper apdu;
		int rv;

		if (isBasicChannel() == false)
		{
			/* close channel */
			command = apdu.generateAPDU(APDUHelper::COMMAND_CLOSE_LOGICAL_CHANNEL, channelNum, ByteArray::EMPTY);
			rv = terminal->transmitSync(command, result);

			if (rv == 0 && result.getLength() >= 2)
			{
				ResponseHelper resp(result);

				if (resp.getStatus() == 0)
				{
					_DBG("close success");
				}
				else
				{
					_ERR("status word [%d][ %02X %02X ]", resp.getStatus(), resp.getSW1(), resp.getSW2());
				}
			}
			else
			{
				_ERR("select apdu is failed, rv [%d], length [%d]", rv, result.getLength());
			}
		}

		channelNum = -1;
	}

	int ServerChannel::transmitSync(ByteArray command, ByteArray &result)
		throw(ErrorIO &, ErrorIllegalState &, ErrorIllegalParameter &, ErrorSecurity &)
	{
		int ret = -1;
		APDUCommand helper;

		if (isClosed() == true)
		{
			return ret;
		}

		helper.setCommand(command);

		/* filter command */
		if (privilege == false)
		{
			if ((helper.getINS() == APDUCommand::INS_SELECT_FILE &&
				helper.getP1() == APDUCommand::P1_SELECT_BY_DF_NAME) ||
				(helper.getINS() == APDUCommand::INS_MANAGE_CHANNEL))
			{
				return -4; /* security reason */
			}
		}

		/* TODO : insert channel ID using atr information */
		helper.setChannel(APDUCommand::CLA_CHANNEL_STANDARD, channelNum);

		helper.getBuffer(command);

		_DBG("command [%d] : %s", command.getLength(), command.toString());

		ret = terminal->transmitSync(command, result);
		if (ret == 0 && ResponseHelper::getStatus(result) == 0)
		{
			/* store select response */
			if (helper.getINS() == APDUCommand::INS_SELECT_FILE)
				setSelectResponse(result);
		}

		return ret;
	}

} /* namespace smartcard_service_api */
