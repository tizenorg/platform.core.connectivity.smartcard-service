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
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#ifdef USE_UNIX_DOMAIN
#include <sys/un.h>
#include <sys/stat.h>
#else /* USE_UNIX_DOMAIN */
#include <netinet/in.h>
#endif /* USE_UNIX_DOMAIN */
#include <fcntl.h>

/* SLP library header */

/* local header */
#include "Debug.h"
#include "IPCHelper.h"

#ifdef USE_UNIX_DOMAIN
#define OMAPI_SERVER_DOMAIN "/tmp/omapi-server-domain"
#endif /* USE_UNIX_DOMAIN */

static void setNonBlockSocket(int socket)
{
	int flags;

	flags = fcntl(socket, F_GETFL);

	flags |= O_NONBLOCK;

	if (fcntl(socket, F_SETFL, flags) < 0)
	{
		/* SCARD_DEBUG_ERR("fcntl, executing nonblock error"); */
	}
}

namespace smartcard_service_api
{
	IPCHelper::IPCHelper()
	{
		ipcSocket = -1;
		ioChannel = NULL;
		watchId = 0;
		memset(&ipcLock, 0, sizeof(ipcLock));
		dispatcher = NULL;
	}

	IPCHelper::~IPCHelper()
	{
	}

	gboolean IPCHelper::channelCallbackFunc(GIOChannel* channel, GIOCondition condition, gpointer data)
	{
		IPCHelper *helper = (IPCHelper *)data;
		gboolean result = FALSE;

		SCARD_DEBUG("channel [%p], condition [%d], data [%p]", channel, condition, data);

		if (helper == NULL)
		{
			SCARD_DEBUG_ERR("ipchelper is null");
			return result;
		}

		if ((G_IO_ERR & condition) || (G_IO_HUP & condition))
		{
			result = helper->handleIOErrorCondition(channel, condition);
		}
		else if (G_IO_NVAL & condition)
		{
			result = helper->handleInvalidSocketCondition(channel, condition);
		}
		else if (G_IO_IN & condition)
		{
			result = helper->handleIncomingCondition(channel, condition);
		}

		return result;
	}

	bool IPCHelper::createListenSocket()
	{
		GIOCondition condition = (GIOCondition)(G_IO_ERR | G_IO_HUP | G_IO_IN);
		struct sockaddr_un saddrun_rv;

		if (ipcSocket >= 0)
			return true;

		memset(&saddrun_rv, 0, sizeof(struct sockaddr_un));

		unlink(OMAPI_SERVER_DOMAIN);

		ipcSocket = socket(AF_UNIX, SOCK_STREAM, 0);
		if (ipcSocket == -1)
		{
			SCARD_DEBUG_ERR("get socket is failed");
			return false;
		}

		::setNonBlockSocket(ipcSocket);

		saddrun_rv.sun_family = AF_UNIX;
		strncpy(saddrun_rv.sun_path, OMAPI_SERVER_DOMAIN, sizeof(saddrun_rv.sun_path) - 1);

		if (bind(ipcSocket, (struct sockaddr *)&saddrun_rv, sizeof(saddrun_rv)) < 0)
		{
			SCARD_DEBUG_ERR("bind is failed \n");
			goto ERROR;
		}

		if (chmod(OMAPI_SERVER_DOMAIN, 0777) < 0)
		{
			SCARD_DEBUG_ERR("can not change permission of UNIX DOMAIN file");
			goto ERROR;
		}

		if (listen(ipcSocket, IPC_SERVER_MAX_CLIENT) < 0)
		{
			SCARD_DEBUG_ERR("listen is failed \n");
			goto ERROR;
		}

		if ((ioChannel = g_io_channel_unix_new(ipcSocket)) != NULL)
		{
			if ((watchId = g_io_add_watch(ioChannel, condition, &IPCHelper::channelCallbackFunc, this)) < 1)
			{
				SCARD_DEBUG_ERR(" g_io_add_watch is failed \n");
				goto ERROR;
			}
		}
		else
		{
			SCARD_DEBUG_ERR(" g_io_channel_unix_new is failed \n");
			goto ERROR;
		}

#ifdef SECURITY_SERVER
		gid = security_server_get_gid(NET_NFC_MANAGER_OBJECT);
		if(gid == 0)
		{
			SCARD_DEBUG("get gid from security server is failed. this object is not allowed by security server");
			goto ERROR;
		}

		if((cookies_size = security_server_get_cookie_size()) != 0)
		{
			if((cookies = (char *)calloc(1, cookies_size)) == NULL)
			{
				goto ERROR;
			}
		}
#endif

		SCARD_DEBUG("server ipc is initialized");

		return true;
ERROR :
		if (watchId != (uint32_t)-1)
		{
			g_source_remove(watchId);
			watchId = -1;
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

		return false;
	}

	bool IPCHelper::createConnectSocket()
	{
		GIOCondition condition = (GIOCondition) (G_IO_ERR | G_IO_HUP | G_IO_IN);

		SCARD_BEGIN();

		if (ipcSocket >= 0)
			return true;

		pthread_mutex_lock(&ipcLock);

		struct sockaddr_un saddrun_rv;
		socklen_t len_saddr = 0;

		memset(&saddrun_rv, 0, sizeof(struct sockaddr_un));

		ipcSocket = socket(AF_UNIX, SOCK_STREAM, 0);
		if (ipcSocket == -1)
		{
			SCARD_DEBUG_ERR("get socket is failed \n");
			pthread_mutex_unlock(&ipcLock);

			SCARD_END();

			return false;
		}

		SCARD_DEBUG("socket is created");

		::setNonBlockSocket(ipcSocket);

		saddrun_rv.sun_family = AF_UNIX;
		strncpy(saddrun_rv.sun_path, OMAPI_SERVER_DOMAIN, sizeof(saddrun_rv.sun_path) - 1);

		len_saddr = sizeof(saddrun_rv.sun_family) + strlen(OMAPI_SERVER_DOMAIN);

		if ((connect(ipcSocket, (struct sockaddr *)&saddrun_rv, len_saddr)) < 0)
		{
			SCARD_DEBUG_ERR("error is occured");
			pthread_mutex_unlock(&ipcLock);
			goto ERROR;
		}

		pthread_mutex_unlock(&ipcLock);

		if ((ioChannel = g_io_channel_unix_new(ipcSocket)) != NULL)
		{
			if ((watchId = g_io_add_watch(ioChannel, condition, &IPCHelper::channelCallbackFunc, this)) < 1)
			{
				SCARD_DEBUG_ERR(" g_io_add_watch is failed \n");
				goto ERROR;
			}
		}
		else
		{
			SCARD_DEBUG_ERR(" g_io_channel_unix_new is failed \n");
			goto ERROR;
		}

		SCARD_DEBUG("socket and g io channel is binded");

		SCARD_END();

		return true;

ERROR :
		SCARD_DEBUG_ERR("error while initializing client ipc");

		if(watchId != 0)
		{
			g_source_remove(watchId);
			watchId = 0;
		}

		if(ioChannel != NULL)
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

		SCARD_END();

		return false;
	}

	bool IPCHelper::sendMessage(Message *msg)
	{
		if (ipcSocket == -1)
			return false;

		return sendMessage(ipcSocket, msg);
	}

	bool IPCHelper::sendMessage(int socket, Message *msg)
	{
		bool result = false;
		ByteArray stream;
		unsigned int length = 0;

		stream = msg->serialize();
		length = stream.getLength();

		SCARD_DEBUG(">>>[SEND]>>> socket [%d], msg [%d], length [%d]", socket, msg->message, length);

		if (length > 0)
		{
			int sentBytes = 0;

			/* send 4 bytes (length) */
			pthread_mutex_lock(&ipcLock);
			sentBytes = send(socket, &length, sizeof(length), 0);
			pthread_mutex_unlock(&ipcLock);
			if (sentBytes == sizeof(length))
			{
				unsigned int current = 0;

				/* send message */
				pthread_mutex_lock(&ipcLock);
				do
				{
					sentBytes = send(socket, stream.getBuffer(current), length - current, 0);
					if (sentBytes > 0)
						current += sentBytes;
				}
				while (current < length);
				pthread_mutex_unlock(&ipcLock);

				result = true;
			}
			else
			{
				SCARD_DEBUG_ERR("send failed, sentBytes [%d]", sentBytes);
			}
		}
		else
		{
			SCARD_DEBUG_ERR("stream length is zero");
		}

		return result;
	}

	Message *IPCHelper::retrieveMessage()
	{
		return retrieveMessage(ipcSocket);
	}

	Message *IPCHelper::retrieveMessage(int socket)
	{
		Message *msg = NULL;
		unsigned int length = 0;
		int readBytes = 0;

		SCARD_BEGIN();

		/* read 4 bytes (length) */
		pthread_mutex_lock(&ipcLock);
		readBytes = recv(socket, &length, sizeof(length), 0);
		pthread_mutex_unlock(&ipcLock);
		if (readBytes == sizeof(length))
		{
			if (length > 0)
			{
				unsigned char *buffer = NULL;

				/* prepare buffer */
				buffer = new unsigned char[length];
				if (buffer != NULL)
				{
					int retry = 0;
					unsigned int current = 0;

					/* read message */
					pthread_mutex_lock(&ipcLock);
					do
					{
						readBytes = recv(socket, buffer + current, length - current, 0);
						if (readBytes > 0)
							current += readBytes;
						retry++;
					}
					while (current < length);
					pthread_mutex_unlock(&ipcLock);

					msg = new Message();
					if (msg != NULL)
					{
						msg->deserialize(buffer, length);

						SCARD_DEBUG("<<<[RETRIEVE]<<< socket [%d], msg_length [%d]", socket, length);
					}
					else
					{
						SCARD_DEBUG_ERR("alloc failed");
					}

					delete []buffer;
				}
				else
				{
					SCARD_DEBUG_ERR("allocation failed");
				}
			}
			else
			{
				SCARD_DEBUG_ERR("invalid length, socket = [%d], msg_length = [%d]", socket, length);
			}
		}
		else
		{
			SCARD_DEBUG_ERR("failed to recv length, socket = [%d], readBytes [%d]", socket, readBytes);
		}


		SCARD_END();

		return msg;
	}

	void IPCHelper::setDispatcher(DispatcherHelper *dispatcher)
	{
		this->dispatcher = dispatcher;
	}

} /* namespace smartcard_service_api */
