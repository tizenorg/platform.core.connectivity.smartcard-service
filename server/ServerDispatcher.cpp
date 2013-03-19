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
#include <string.h>

/* SLP library header */

/* local header */
#include "Debug.h"
#include "Exception.h"
#include "ServerDispatcher.h"
#include "ServerResource.h"
#include "ServerSEService.h"
#include "ServerChannel.h"
#include "ServerSession.h"
#include "ServerReader.h"

namespace smartcard_service_api
{
	ServerDispatcher::ServerDispatcher():DispatcherHelper()
	{
		SCARD_BEGIN();

		runDispatcherThread();

		SCARD_END();
	}

	ServerDispatcher::~ServerDispatcher()
	{
	}

	ServerDispatcher *ServerDispatcher::getInstance()
	{
		static ServerDispatcher instance;

		return &instance;
	}

	void *ServerDispatcher::dispatcherThreadFunc(DispatcherMsg *msg, void *data)
	{
		int socket = -1;
		ServerResource *resource = NULL;

		if (data == NULL)
		{
			SCARD_DEBUG_ERR("dispatcher instance is null");
			return NULL;
		}

		if (msg == NULL)
		{
			SCARD_DEBUG_ERR("message is null");
			return NULL;
		}

		resource = &ServerResource::getInstance();
		socket = msg->getPeerSocket();

		switch (msg->message)
		{
		/* handle message */
		case Message::MSG_REQUEST_READERS :
			{
				SCARD_DEBUG("[MSG_REQUEST_READERS]");

#if 0
				seService->dispatcherCallback(msg, msg->getPeerSocket());
#else
				int count = 0;
				Message response(*msg);
				ByteArray info;
				ClientInstance *instance = NULL;

				response.param1 = 0;
				response.param2 = 0;

				/* load secure elements */
				resource->loadSecureElements();

				if ((instance = resource->getClient(socket)) != NULL)
				{
					/* update client PID */
					if (instance->getPID() == -1)
					{
						instance->setPID(msg->error);
						SCARD_DEBUG_ERR("update PID [%d]", msg->error);

						/* generate certification hashes */
						instance->generateCertificationHashes();
					}

					/* create service */
					if (resource->createService(socket, (unsigned int)msg->userParam) != NULL)
					{
						response.error = SCARD_ERROR_OK;

						if ((count = resource->getReadersInformation(info)) > 0)
						{
							response.param1 = count;
							response.data = info;
						}
						else
						{
							SCARD_DEBUG("no secure elements");
						}
					}
					else
					{
						SCARD_DEBUG_ERR("createClient failed");

						response.error = SCARD_ERROR_UNAVAILABLE;
					}
				}
				else
				{
					SCARD_DEBUG("client doesn't exist, socket [%d]", socket);

					response.error = SCARD_ERROR_UNAVAILABLE;
				}

				/* response to client */
				ServerIPC::getInstance()->sendMessage(socket, &response);
#endif
			}
			break;

		case Message::MSG_REQUEST_SHUTDOWN :
#if 0
			{
				Message response(*msg);

				SCARD_DEBUG("[MSG_REQUEST_SHUTDOWN]");

				if (msg->param1 != 0)
				{
					ServerChannel *channel = NULL;

					channel = (ServerChannel *)msg->param1;

					channel->closeSync();
				}

				/* response to client */
				ServerIPC::getInstance()->sendMessage(msg->getPeerSocket(), &response);
			}
#else
			{
				Message response(*msg);

				SCARD_DEBUG("[MSG_REQUEST_SHUTDOWN]");

				response.error = SCARD_ERROR_OK;

				resource->removeService(socket, msg->error/* service context */);

				/* response to client */
				ServerIPC::getInstance()->sendMessage(socket, &response);
			}
#endif
			break;

		case Message::MSG_REQUEST_OPEN_SESSION :
#if 0
			{
				Message response(*msg);
				ServerReader *reader = NULL;
				ServerSession *session = NULL;

				SCARD_DEBUG("[MSG_REQUEST_OPEN_SESSION]");

				if (msg->param1 != 0)
				{
					reader = (ServerReader *)msg->param1;

					session = reader->openSessionSync(msg->data, msg->caller);
				}

				/* TODO : attach atr??? */
				response.param1 = (unsigned int)session;

				/* response to client */
				ServerIPC::getInstance()->sendMessage(msg->getPeerSocket(), &response);
			}
#else
			{
				Message response(*msg);
				unsigned int handle = IntegerHandle::INVALID_HANDLE;

				SCARD_DEBUG("[MSG_REQUEST_OPEN_SESSION]");

				if (resource->isValidReaderHandle(msg->param1))
				{
					vector<ByteArray> temp;

					handle = resource->createSession(socket, msg->error/* service context */, msg->param1, temp, msg->caller);
					if (handle != IntegerHandle::INVALID_HANDLE)
					{
						response.error = SCARD_ERROR_OK;
					}
					else
					{
						SCARD_DEBUG_ERR("createSession failed [%d]", handle);
						response.error = SCARD_ERROR_OUT_OF_MEMORY;
					}
				}
				else
				{
					SCARD_DEBUG_ERR("request invalid reader handle [%d]", msg->param1);
					response.error = SCARD_ERROR_ILLEGAL_PARAM;
				}

				response.param1 = handle;

				/* response to client */
				ServerIPC::getInstance()->sendMessage(socket, &response);
			}
#endif
			break;

		case Message::MSG_REQUEST_CLOSE_SESSION :
#if 0
			{
				Message response(*msg);
				ServerSession *session = NULL;

				SCARD_DEBUG("[MSG_REQUEST_CLOSE_SESSION]");

				if (msg->param1 != 0)
				{
					session = (ServerSession *)msg->param1;

					session->closeSync();
				}

				/* response to client */
				ServerIPC::getInstance()->sendMessage(msg->getPeerSocket(), &response);
			}
#else
			{
				Message response(*msg);

				SCARD_DEBUG("[MSG_REQUEST_CLOSE_SESSION]");

				response.param1 = 0;
				response.error = SCARD_ERROR_OK;

				if (resource->isValidSessionHandle(socket, msg->error/* service context */, msg->param1))
				{
					resource->removeSession(socket, msg->error/* service context */, msg->param1);
				}

				/* response to client */
				ServerIPC::getInstance()->sendMessage(socket, &response);
			}
#endif
			break;

		case Message::MSG_REQUEST_OPEN_CHANNEL :
#if 0
			{
				Message response(*msg);
				ServerSession *session = NULL;

				SCARD_DEBUG("[MSG_REQUEST_OPEN_CHANNEL]");

				if (/* check valid session handle */msg->param2 != 0)
				{
					ServerChannel *channel = NULL;

					session = (ServerSession *)msg->param2;

					if (msg->param1 == 0)
						channel = (ServerChannel *)session->openBasicChannelSync(msg->data, msg->caller);
					else
						channel = (ServerChannel *)session->openLogicalChannelSync(msg->data, msg->caller);

					if (channel != NULL)
					{
						response.param1 = (unsigned int)channel;
						response.param2 = channel->getChannelID();
						response.error = 0;
						response.data = channel->getSelectResponse();
					}
					else
					{
						SCARD_DEBUG_ERR("channel is null.");

						/* set error value */
						response.param1 = 0;
						response.param2 = 0;
						response.error = -4;
						response.data.releaseBuffer();
					}
				}
				else
				{
					SCARD_DEBUG_ERR("session is invalid");

					response.param1 = 0;
					response.param2 = 0;
					response.error = -1;
					response.data.releaseBuffer();
				}

				/* response to client */
				ServerIPC::getInstance()->sendMessage(msg->getPeerSocket(), &response);
			}
#else
			{
				Message response(*msg);

				SCARD_DEBUG("[MSG_REQUEST_OPEN_CHANNEL]");

				response.param1 = IntegerHandle::INVALID_HANDLE;
				response.param2 = 0;
				response.data.releaseBuffer();

				try
				{
					unsigned int channelID = IntegerHandle::INVALID_HANDLE;

					channelID = resource->createChannel(socket, msg->error/* service context */, msg->param2, msg->param1, msg->data);
					if (channelID != IntegerHandle::INVALID_HANDLE)
					{
						ServerChannel *temp;

						temp = (ServerChannel *)resource->getChannel(socket, msg->error/* service context */, channelID);
						if (temp != NULL)
						{
							response.param1 = channelID;
							response.param2 = temp->getChannelNumber();
							response.error = SCARD_ERROR_OK;
							response.data = temp->getSelectResponse();
						}
						else
						{
							SCARD_DEBUG_ERR("IS IT POSSIBLE??????????????????");
							response.error = SCARD_ERROR_UNKNOWN;
						}
					}
					else
					{
						SCARD_DEBUG_ERR("channel is null.");

						/* set error value */
						response.error = SCARD_ERROR_UNAVAILABLE;
					}
				}
				catch (ExceptionBase &e)
				{
					response.error = e.getErrorCode();
				}

				/* response to client */
				ServerIPC::getInstance()->sendMessage(socket, &response);
			}
#endif
			break;

		case Message::MSG_REQUEST_GET_CHANNEL_COUNT :
			{
				Message response(*msg);

				SCARD_DEBUG("[MSG_REQUEST_GET_CHANNEL_COUNT]");

				response.error = SCARD_ERROR_OK;
				response.param1 = resource->getChannelCount(socket, msg->error/* service context */, msg->param1);

				/* response to client */
				ServerIPC::getInstance()->sendMessage(socket, &response);
			}
			break;

		case Message::MSG_REQUEST_CLOSE_CHANNEL :
#if 0
			{
				Message response(*msg);

				SCARD_DEBUG("[MSG_REQUEST_CLOSE_CHANNEL]");

				if (msg->param1 != 0)
				{
					ServerChannel *channel = NULL;

					channel = (ServerChannel *)msg->param1;

					channel->closeSync();
				}

				/* response to client */
				ServerIPC::getInstance()->sendMessage(msg->getPeerSocket(), &response);
			}
#else
			{
				Message response(*msg);

				SCARD_DEBUG("[MSG_REQUEST_CLOSE_CHANNEL]");

				response.error = SCARD_ERROR_OK;

				if (resource->getChannel(socket, msg->error/* service context */, msg->param1) != NULL)
				{
					resource->removeChannel(socket, msg->error/* service context */, msg->param1);
				}

				/* response to client */
				ServerIPC::getInstance()->sendMessage(socket, &response);
			}
#endif
			break;

		case Message::MSG_REQUEST_GET_ATR :
#if 0
			{
				Message response(*msg);

				SCARD_DEBUG("[MSG_REQUEST_GET_ATR]");

				if (msg->param1 != 0)
				{
					ServerChannel *channel = NULL;

					channel = (ServerChannel *)msg->param1;

					channel->closeSync();
				}

				/* response to client */
				ServerIPC::getInstance()->sendMessage(msg->getPeerSocket(), &response);
			}
#else
			{
				int rv;
				Message response(*msg);
				ByteArray result;
				ServiceInstance *client = NULL;

				SCARD_DEBUG("[MSG_REQUEST_GET_ATR]");

				if ((client = resource->getService(socket, msg->error/* service context */)) != NULL)
				{
					Terminal *terminal = NULL;

					if ((terminal = client->getTerminal(msg->param1)) != NULL)
					{
						if ((rv = terminal->getATRSync(result)) == 0)
						{
							response.data = result;
							response.error = SCARD_ERROR_OK;
						}
						else
						{
							SCARD_DEBUG_ERR("transmit failed [%d]", rv);

							response.error = rv;
						}
					}
					else
					{
						SCARD_DEBUG_ERR("getTerminal failed : socket [%d], context [%d], session [%d]", socket, msg->error/* service context */, msg->param1);
						response.error = SCARD_ERROR_UNAVAILABLE;
					}
				}
				else
				{
					SCARD_DEBUG_ERR("getClient failed : socket [%d], context [%d], session [%d]", socket, msg->error/* service context */, msg->param1);
					response.error = SCARD_ERROR_UNAVAILABLE;
				}

				/* response to client */
				ServerIPC::getInstance()->sendMessage(socket, &response);
			}
#endif
			break;

		case Message::MSG_REQUEST_TRANSMIT :
#if 0
			{
				Message response(*msg);
				ByteArray result;
				int rv;

				SCARD_DEBUG("[MSG_REQUEST_TRANSMIT]");

				if (msg->param1 != 0)
				{
					ServerChannel *channel = NULL;

					channel = (ServerChannel *)msg->param1;

					if ((rv = channel->transmitSync(msg->data, result)) == 0)
					{
						response.data = result;
					}
					else
					{
						SCARD_DEBUG_ERR("transmit failed [%d]", rv);
					}
				}

//				if (resource->isValidChannelHandle((void *)msg->param1))
//				{
//				}

				/* response to client */
				ServerIPC::getInstance()->sendMessage(msg->getPeerSocket(), &response);
			}
#else
			{
				int rv;
				Message response(*msg);
				ByteArray result;
				Channel *channel = NULL;

				SCARD_DEBUG("[MSG_REQUEST_TRANSMIT]");

				if ((channel = resource->getChannel(socket, msg->error/* service context */, msg->param1)) != NULL)
				{
					if ((rv = channel->transmitSync(msg->data, result)) == 0)
					{
						response.data = result;
						response.error = SCARD_ERROR_OK;
					}
					else
					{
						SCARD_DEBUG_ERR("transmit failed [%d]", rv);

						response.error = rv;
					}
				}
				else
				{
					SCARD_DEBUG_ERR("invalid handle : socket [%d], context [%d], channel [%d]", socket, msg->error/* service context */, msg->param1);
					response.error = SCARD_ERROR_UNAVAILABLE;
				}

				/* response to client */
				ServerIPC::getInstance()->sendMessage(socket, &response);
			}
#endif
			break;

		case Message::MSG_OPERATION_RELEASE_CLIENT :
#if 0
			{
				Message response(*msg);

				SCARD_DEBUG("[MSG_REQUEST_CLOSE_CHANNEL]");

				if (msg->param1 != 0)
				{
					ServerChannel *channel = NULL;

					channel = (ServerChannel *)msg->param1;

					channel->closeSync();
				}

				/* response to client */
				ServerIPC::getInstance()->sendMessage(msg->getPeerSocket(), &response);
			}
#else
			{
				SCARD_DEBUG("[MSG_OPERATION_RELEASE_CLIENT]");

				resource->removeClient(msg->param1);
				SCARD_DEBUG("remain client [%d]", resource->getClientCount());
			}
#endif
#ifdef USE_AUTOSTART
			if (resource->getClientCount() == 0)
			{
				SCARD_DEBUG("There is no client. shutting down service");
				g_main_loop_quit((GMainLoop *)resource->getMainLoopInstance());
			}
#endif
			break;

		default :
			SCARD_DEBUG("unknown message [%s], socket [%d]", msg->toString(), socket);
			break;
		}

		return NULL;
	}

} /* namespace smartcard_service_api */
