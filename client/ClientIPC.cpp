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


/* standard library header */
#include <sys/socket.h>
#include <unistd.h>

/* SLP library header */

/* local header */
#include "Debug.h"
#include "ClientIPC.h"
#include "DispatcherMsg.h"

namespace smartcard_service_api
{
	ClientIPC::ClientIPC():IPCHelper()
	{
	}

	ClientIPC::~ClientIPC()
	{
	}

	ClientIPC &ClientIPC::getInstance()
	{
		static ClientIPC clientIPC;

		return clientIPC;
	}

	int ClientIPC::handleIOErrorCondition(void *channel, GIOCondition condition)
	{
		SCARD_BEGIN();

		/* finalize context */
		if (watchId != 0)
		{
			g_source_remove(watchId);
			watchId = 0;
		}

		if (ioChannel != NULL)
		{
			g_io_channel_unref(ioChannel);
			ioChannel = NULL;
		}

		if (ipcSocket != -1)
		{
			shutdown(ipcSocket, SHUT_RDWR);
			close(ipcSocket);

			ipcSocket = -1;
		}

		/* push disconnect message */
		DispatcherMsg *dispMsg = new DispatcherMsg();

		dispMsg->message = Message::MSG_OPERATION_RELEASE_CLIENT;
		dispMsg->error = -1;

		if (dispatcher != NULL)
			dispatcher->pushMessage(dispMsg);

		SCARD_END();

		return FALSE;
	}

	int ClientIPC::handleInvalidSocketCondition(void *channel, GIOCondition condition)
	{
		SCARD_BEGIN();

		/* finalize context */

		SCARD_END();

		return FALSE;
	}

	int ClientIPC::handleIncomingCondition(void *channel, GIOCondition condition)
	{
		int result = FALSE;

		SCARD_BEGIN();

		if (channel == ioChannel)
		{
			Message *msg = NULL;

			SCARD_DEBUG("message from server to client socket");

			/* read message */
			msg = retrieveMessage();
			if (msg != NULL)
			{
				DispatcherMsg *dispMsg = new DispatcherMsg(msg);

				/* set peer socket */
				dispMsg->setPeerSocket(ipcSocket);

				/* push to dispatcher */
				if (dispatcher != NULL)
					dispatcher->pushMessage(dispMsg);

				result = TRUE;
			}
			else
			{
				/* clear client connection */
			}

			delete msg;
		}
		else
		{
			SCARD_DEBUG_ERR("Unknown channel event [%p]", channel);
		}

		SCARD_END();

		return result;
	}

} /* namespace open_mobile_api */
