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
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#ifdef USE_UNIX_DOMAIN
#include <sys/un.h>
#include <sys/stat.h>
#else /* USE_UNIX_DOMAIN */
#include <netinet/in.h>
#endif /* USE_UNIX_DOMAIN */
#include <fcntl.h>
#ifdef USE_IPC_EPOLL
#include <sys/epoll.h>
#endif

/* SLP library header */

/* local header */
#include "Debug.h"
#include "IPCHelper.h"

#ifdef USE_UNIX_DOMAIN
#define SCARD_SERVER_DOMAIN "/tmp/smartcard-server-domain"
#endif /* USE_UNIX_DOMAIN */

static void setNonBlockSocket(int socket)
{
	int flags;

	flags = fcntl(socket, F_GETFL);

	flags |= O_NONBLOCK;

	if (fcntl(socket, F_SETFL, flags) < 0)
	{
		/* _ERR("fcntl, executing nonblock error"); */
	}
}

namespace smartcard_service_api
{
	IPCHelper::IPCHelper() : fdPoll(-1)
	{
		ipcSocket = -1;
		ioChannel = NULL;
		watchId = 0;
		memset(&ipcLock, 0, sizeof(ipcLock));
		dispatcher = NULL;
		pollEvents = NULL;
		readThread = 0;
	}

	IPCHelper::~IPCHelper()
	{
	}

	gboolean IPCHelper::channelCallbackFunc(GIOChannel* channel, GIOCondition condition, gpointer data)
	{
		IPCHelper *helper = (IPCHelper *)data;
		gboolean result = FALSE;

		_DBG("channel [%p], condition [%d], data [%p]", channel, condition, data);

		if (helper == NULL)
		{
			_ERR("ipchelper is null");
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

		unlink(SCARD_SERVER_DOMAIN);

		ipcSocket = socket(AF_UNIX, SOCK_STREAM, 0);
		if (ipcSocket == -1)
		{
			_ERR("get socket is failed");
			return false;
		}

		::setNonBlockSocket(ipcSocket);

		saddrun_rv.sun_family = AF_UNIX;
		strncpy(saddrun_rv.sun_path, SCARD_SERVER_DOMAIN, sizeof(saddrun_rv.sun_path) - 1);

		if (bind(ipcSocket, (struct sockaddr *)&saddrun_rv, sizeof(saddrun_rv)) < 0)
		{
			_ERR("bind is failed");
			goto ERROR;
		}

		if (chmod(SCARD_SERVER_DOMAIN, 0777) < 0)
		{
			_ERR("can not change permission of UNIX DOMAIN file");
			goto ERROR;
		}

		if (listen(ipcSocket, IPC_SERVER_MAX_CLIENT) < 0)
		{
			_ERR("listen is failed");
			goto ERROR;
		}

		if ((ioChannel = g_io_channel_unix_new(ipcSocket)) != NULL)
		{
			if ((watchId = g_io_add_watch(ioChannel, condition, &IPCHelper::channelCallbackFunc, this)) < 1)
			{
				_ERR(" g_io_add_watch is failed");
				goto ERROR;
			}
		}
		else
		{
			_ERR(" g_io_channel_unix_new is failed");
			goto ERROR;
		}

#ifdef SECURITY_SERVER
		int gid, cookies_size;
		char *cookies;

		gid = security_server_get_gid("smartcard-service");
		if(gid == 0)
		{
			_DBG("get gid from security server is failed. this object is not allowed by security server");
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

		_INFO("server ipc is initialized");

		return true;
ERROR :
		_ERR("error while initializing server ipc");

		destroyListenSocket();

		return false;
	}

#ifdef CLIENT_IPC_THREAD
	int IPCHelper::eventPoll()
	{
		int result = -1;

#ifdef USE_IPC_EPOLL
		int events = 0;

		if ((events = epoll_wait(fdPoll, pollEvents, EPOLL_SIZE, -1)) > 0)
		{
			int i;

			for (i = 0; i < events; i++)
			{
				_DBG("pollEvents[%d].events [%X]", i, pollEvents[i].events);

				if ((pollEvents[i].events & EPOLLHUP) || (pollEvents[i].events & EPOLLERR))
				{
					_ERR("connection is closed");
					result = 0;
					break;
				}
				else if (pollEvents[i].events & EPOLLIN)
				{
					result = 1;
					break;
				}
			}
		}
		else if (errno == EINTR)
		{
			_ERR("epoll_wait interrupted");
		}
		else
		{
			char buffer[1024];

			_ERR("epoll_wait failed, errno [%d], %s", errno, strerror_r(errno, buffer, sizeof(buffer)));
		}
#else
		if (select(ipcSocket + 1, &fdSetRead, NULL, NULL, NULL) > 0)
		{
			if (FD_ISSET(ipcSocket, &fdSetRead) == true)
			{
				int val = -1;
				unsigned int size = sizeof(val);

				if (getsockopt(ipcSocket, SOL_SOCKET, SO_ERROR, (void *)&val, &size) == 0)
				{
					if (val == 0)
					{
						_DBG("socket is readable");
						result = 1;
					}
					else
					{
						_DBG("socket is not available. maybe disconnected");
						result = 0;
					}
				}
				else
				{
					_ERR("getsockopt failed, errno [%d]", errno);
					result = errno;
				}
			}
			else
			{
				_ERR("FD_ISSET false!!! what's wrong");
				result = -1;
			}
		}
		else
		{
			_ERR("select failed [%d]", errno);
			result = errno;
		}
#endif
		return result;
	}

	void *IPCHelper::threadRead(void *data)
	{
#ifdef IPC_USE_SIGTERM

		struct sigaction act;
		act.sa_handler = thread_sig_handler;
		sigaction(SIGTERM, &act, NULL);

		sigset_t newmask;
		sigemptyset(&newmask);
		sigaddset(&newmask, SIGTERM);
		pthread_sigmask(SIG_UNBLOCK, &newmask, NULL);
		_DBG("sighandler is registered");

		pthread_mutex_lock(&g_client_lock);
		pthread_cond_signal ((pthread_cond_t *) data);
		pthread_mutex_unlock(&g_client_lock);
#else
		IPCHelper *helper = (IPCHelper *)data;
#endif
		bool condition = true;
		int result = 0;

		pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);

		while (condition == true)
		{
			if ((result = helper->eventPoll()) > 0)
			{
				condition = (helper->handleIncomingCondition(NULL, G_IO_IN) == 0);
			}
			else if (result == 0)
			{
				helper->handleIOErrorCondition(NULL, G_IO_ERR);
				condition = false;
			}
			else
			{
				/* skip other error case */
			}
		}

		_INFO("threadRead is terminated");

		return (void *)NULL;
	}
#endif

	bool IPCHelper::createConnectSocket()
	{
#ifndef CLIENT_IPC_THREAD
		GIOCondition condition = (GIOCondition)(G_IO_ERR | G_IO_HUP | G_IO_IN);
#endif
		int result = 0;
		char err[200] = { 0, };

		_BEGIN();

		if (ipcSocket >= 0)
			return true;

		pthread_mutex_lock(&ipcLock);

		struct sockaddr_un saddrun_rv;
		socklen_t len_saddr = 0;

		memset(&saddrun_rv, 0, sizeof(struct sockaddr_un));

		ipcSocket = socket(AF_UNIX, SOCK_STREAM, 0);
		if (ipcSocket == -1)
		{
			_ERR("get socket is failed [%d, %s]",
				errno, strerror_r(errno, err, sizeof(err)));
			goto ERROR;
		}

		_DBG("socket is created");

		::setNonBlockSocket(ipcSocket);

		saddrun_rv.sun_family = AF_UNIX;
		strncpy(saddrun_rv.sun_path, SCARD_SERVER_DOMAIN, sizeof(saddrun_rv.sun_path) - 1);

		len_saddr = sizeof(saddrun_rv.sun_family) + strlen(SCARD_SERVER_DOMAIN);

		if ((result = connect(ipcSocket, (struct sockaddr *)&saddrun_rv, len_saddr)) < 0)
		{
			_ERR("connect failed [%d, %s]",
				errno, strerror_r(errno, err, sizeof(err)));
			goto ERROR;
		}

#ifdef CLIENT_IPC_THREAD
#ifdef USE_IPC_EPOLL
		if((fdPoll = epoll_create1(EPOLL_CLOEXEC)) == -1)
		{
			_ERR("epoll_create1 failed [%d, %s]",
				errno, strerror_r(errno, err, sizeof(err)));
			goto ERROR;
		}

		pollEvents = (struct epoll_event *)calloc(EPOLL_SIZE, sizeof(struct epoll_event));
		if (pollEvents == NULL)
		{
			_ERR("alloc failed");
			goto ERROR;
		}

		struct epoll_event ev;

		ev.events = EPOLLIN | EPOLLHUP | EPOLLERR;
		ev.data.fd = ipcSocket;

		epoll_ctl(fdPoll, EPOLL_CTL_ADD, ipcSocket, &ev);
#else
		FD_ZERO(&fdSetRead);
		FD_SET(ipcSocket, &fdSetRead);
#endif
#ifdef IPC_USE_SIGTERM
		pthread_cond_t pcond = PTHREAD_COND_INITIALIZER;

		if (pthread_create(&readThread, NULL, &IPCHelper::threadRead, &pcond) != 0)
#else
		if (pthread_create(&readThread, NULL, &IPCHelper::threadRead, this) != 0)
#endif
		{
			_ERR("pthread_create is failed");
			goto ERROR;
		}

#ifdef IPC_USE_SIGTERM
		pthread_cond_wait (&pcond, &g_client_lock);
#endif

#else
		if ((ioChannel = g_io_channel_unix_new(ipcSocket)) != NULL)
		{
			if ((watchId = g_io_add_watch(ioChannel, condition, &IPCHelper::channelCallbackFunc, this)) < 1)
			{
				_ERR(" g_io_add_watch is failed");
				goto ERROR;
			}
		}
		else
		{
			_ERR(" g_io_channel_unix_new is failed");
			goto ERROR;
		}
#endif
		pthread_mutex_unlock(&ipcLock);

		_INFO("connecting success");

		_END();

		return true;

ERROR :
		_ERR("error while initializing client ipc");

		destroyConnectSocket();

		pthread_mutex_unlock(&ipcLock);

		_END();

		return false;
	}

	void IPCHelper::destroyListenSocket()
	{
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
	}

	void IPCHelper::destroyConnectSocket()
	{
#ifdef CLIENT_IPC_THREAD
		/* kill thread */
		if (readThread > 0)
		{
			pthread_cancel(readThread);
			readThread = 0;
		}
#ifdef USE_IPC_EPOLL
		if(fdPoll != -1)
		{
			struct epoll_event ev;

			ev.events = EPOLLIN | EPOLLHUP | EPOLLERR;
			ev.data.fd = ipcSocket;
			epoll_ctl(fdPoll, EPOLL_CTL_DEL, ipcSocket, &ev);

			close(fdPoll);
			fdPoll = -1;

			if (pollEvents != NULL)
			{
				free(pollEvents);
			}
		}
#else
		if (ipcSocket != -1)
		{
			FD_CLR(ipcSocket, &fdSetRead);
		}
#endif
#else
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
#endif

		if (ipcSocket != -1)
		{
			shutdown(ipcSocket, SHUT_RDWR);
			close(ipcSocket);
			ipcSocket = -1;
		}
	}

	bool IPCHelper::sendMessage(int socket, const Message &msg)
	{
		ByteArray stream;
		unsigned int length = 0;

		stream = msg.serialize();
		length = stream.size();

		_DBG(">>>[SEND]>>> socket [%d], msg [%d], length [%d]",
			socket, msg.message, stream.size());

		return sendMessage(socket, stream);
	}

	bool IPCHelper::sendMessage(int socket, const ByteArray &buffer)
	{
		bool result = false;
		unsigned int length = 0;

		length = buffer.size();

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
					sentBytes = send(socket, buffer.getBuffer(current), length - current, 0);
					if (sentBytes > 0)
						current += sentBytes;
				}
				while (current < length);
				pthread_mutex_unlock(&ipcLock);

				result = true;
			}
			else
			{
				_ERR("send failed, sentBytes [%d]", sentBytes);
			}
		}
		else
		{
			_ERR("stream length is zero");
		}

		return result;
	}

	Message *IPCHelper::retrieveMessage()
	{
		return retrieveMessage(ipcSocket);
	}

	Message *IPCHelper::retrieveMessage(int socket)
	{
		ByteArray buffer;
		Message *msg = NULL;

		_BEGIN();

		buffer = retrieveBuffer(socket);
		if (buffer.size() > 0)
		{
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

	const ByteArray IPCHelper::retrieveBuffer(int socket)
	{
		ByteArray buffer;
		unsigned int length = 0;
		int readBytes = 0;

		_BEGIN();

		/* read 4 bytes (length) */
		pthread_mutex_lock(&ipcLock);
		readBytes = recv(socket, &length, sizeof(length), 0);
		pthread_mutex_unlock(&ipcLock);
		if (readBytes == sizeof(length))
		{
			if (length > 0)
			{
				uint8_t *temp = NULL;

				/* prepare buffer */
				temp = new uint8_t[length];
				if (temp != NULL)
				{
					int retry = 0;
					unsigned int current = 0;

					/* read message */
					pthread_mutex_lock(&ipcLock);
					do
					{
						readBytes = recv(socket, temp + current, length - current, 0);
						if (readBytes > 0)
							current += readBytes;
						retry++;
					}
					while (current < length);
					pthread_mutex_unlock(&ipcLock);

					_DBG("<<<[RETRIEVE]<<< socket [%d], msg_length [%d]", socket, length);

					buffer.assign(temp, length);

					delete []temp;
				}
				else
				{
					_ERR("allocation failed");
				}
			}
			else
			{
				_ERR("invalid length, socket = [%d], msg_length = [%d]", socket, length);
			}
		}
		else
		{
			_ERR("failed to recv length, socket = [%d], readBytes [%d]", socket, readBytes);
		}

		_END();

		return buffer;
	}

	void IPCHelper::setDispatcher(DispatcherHelper *dispatcher)
	{
		this->dispatcher = dispatcher;
	}

} /* namespace smartcard_service_api */
#endif
