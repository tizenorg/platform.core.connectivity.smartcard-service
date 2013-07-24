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

#ifndef USE_GDBUS
/* standard library header */
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

/* SLP library header */
#ifdef SECURITY_SERVER
#include "security-server.h"
#endif

/* local header */
#include "Debug.h"
#include "ServerIPC.h"
#include "ServerResource.h"
#include "ServerDispatcher.h"

#ifndef EXTERN_API
#define EXTERN_API __attribute__((visibility("default")))
#endif

namespace smartcard_service_api
{
	ServerIPC::ServerIPC() : IPCHelper()
	{
		_BEGIN();

		setDispatcher(ServerDispatcher::getInstance());

		_END();
	}

	ServerIPC::~ServerIPC()
	{
	}

	ServerIPC *ServerIPC::getInstance()
	{
		static ServerIPC instance;

		return &instance;
	}

	Message *ServerIPC::retrieveMessage(int socket)
	{
		ByteArray buffer;
		Message *msg = NULL;

		_BEGIN();

		buffer = IPCHelper::retrieveBuffer(socket);
		if (buffer.getLength() > 0)
		{
#ifdef SECURITY_SERVER
			ByteArray cookie;
			int result, gid;

			if (buffer.size() < 20)
				return msg;

			cookie.assign(buffer.getBuffer(), 20);

			gid = security_server_get_gid("smartcard-daemon");
			if ((result = security_server_check_privilege(cookie.getBuffer(), gid)) != SECURITY_SERVER_API_SUCCESS)
			{
				_ERR("security_server_check_privilege failed [%d]", result);
				return msg;
			}
#endif
			msg = new Message();
			if (msg != NULL)
			{
				msg->deserialize(buffer);
			}
			else
			{
				_ERR("alloc failed");
			}
		}
		else
		{
			_ERR("retrieveBuffer failed ");
		}

		_END();

		return msg;
	}

	bool ServerIPC::acceptClient()
	{
		GIOCondition condition = (GIOCondition)(G_IO_ERR | G_IO_HUP | G_IO_IN);
		socklen_t addrlen = 0;
		int client_sock_fd = 0;
		GIOChannel *client_channel = NULL;
		int client_src_id;

		_DBG("client is trying to connect to server");

		pthread_mutex_lock(&ipcLock);
		client_sock_fd = accept(ipcSocket, NULL, &addrlen);
		pthread_mutex_unlock(&ipcLock);

		if (client_sock_fd < 0)
		{
			_ERR("can not accept client");
			goto ERROR;
		}

		_DBG("client is accepted by server");

		if ((client_channel = g_io_channel_unix_new(client_sock_fd)) == NULL)
		{
			_ERR("create new g io channel is failed");
			goto ERROR;
		}

		if ((client_src_id = g_io_add_watch(client_channel, condition, &IPCHelper::channelCallbackFunc, this)) < 1)
		{
			_ERR("add io callback is failed");
			goto ERROR;
		}

		_INFO("client socket is bond with g_io_channel");

		if (ServerResource::getInstance().createClient(client_channel, client_sock_fd, client_src_id, 0, -1) == false)
		{
			_ERR("failed to add client");
		}

		return true;

ERROR :
		if (client_channel != NULL)
		{
			g_io_channel_unref(client_channel);
		}

		if (client_sock_fd != -1)
		{
			shutdown(client_sock_fd, SHUT_RDWR);
			close(client_sock_fd);
		}

		return false;
	}

	void ServerIPC::restartServerIPC()
	{
		destroyListenSocket();

		createListenSocket();
	}

	void ServerIPC::releaseClient(void *channel, int socket, int watchID)
	{
		if (watchID != 0)
		{
			g_source_remove(watchID);
		}

		if (channel != NULL)
		{
			g_io_channel_unref((GIOChannel *)channel);
		}

		if (socket >= 0)
		{
			shutdown(socket, SHUT_RDWR);
			close(socket);
		}
	}

	int ServerIPC::handleIOErrorCondition(void *channel, GIOCondition condition)
	{
		_BEGIN();

		if(channel == ioChannel)
		{
			_INFO("server socket is closed");
			restartServerIPC();
		}
		else
		{
			DispatcherMsg dispMsg;
			int peerSocket = g_io_channel_unix_get_fd((GIOChannel *)channel);

			_INFO("client socket is closed, socket [%d]", peerSocket);

			/* push message to dispatcher */
			dispMsg.message = Message::MSG_OPERATION_RELEASE_CLIENT;
			dispMsg.param1 = peerSocket;
			dispMsg.setPeerSocket(peerSocket);

			/* push to dispatcher */
			ServerDispatcher::getInstance()->pushMessage(dispMsg);
		}

		_END();

		return FALSE;
	}

	int ServerIPC::handleInvalidSocketCondition(void *channel, GIOCondition condition)
	{
		_BEGIN();
		_END();

		return FALSE;
	}

	int ServerIPC::handleIncomingCondition(void *channel, GIOCondition condition)
	{
		int result = FALSE;

		_BEGIN();

		if(channel == ioChannel)
		{
			/* connect state. should accept */
			_INFO("new client connected");

			result = acceptClient();
		}
		else
		{
			int peerSocket = g_io_channel_unix_get_fd((GIOChannel *)channel);

			_DBG("data incoming from [%d]", peerSocket);

			if (peerSocket >= 0)
			{
				Message *msg;

				/* read message */
				if ((msg = retrieveMessage(peerSocket)) != NULL)
				{
					DispatcherMsg dispMsg(*msg, peerSocket);

					/* push to dispatcher */
					ServerDispatcher::getInstance()->pushMessage(dispMsg);

					result = TRUE;

					delete msg;
				}
				else
				{
					/* clear client connection */
					_ERR("retrieve message failed, socket [%d]", peerSocket);
				}
			}
			else
			{
				_ERR("client context doesn't exist, socket [%d]", peerSocket);
			}
		}

		_END();

		return result;
	}

} /* namespace smartcard_service_api */

using namespace smartcard_service_api;

EXTERN_API void server_ipc_create_listen_socket()
{
	ServerIPC *ipc = ServerIPC::getInstance();

	ipc->createListenSocket();
}
#endif /* USE_GDBUS */
