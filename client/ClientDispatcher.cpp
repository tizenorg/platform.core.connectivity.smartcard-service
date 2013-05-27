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
#include <glib.h>

/* SLP library header */

/* local header */
#include "Debug.h"
#include "ClientDispatcher.h"
#include "SEService.h"
#include "Reader.h"
#include "Session.h"
#include "ClientChannel.h"

namespace smartcard_service_api
{
	ClientDispatcher::ClientDispatcher()
	{
	}

	ClientDispatcher::~ClientDispatcher()
	{
		mapSESerivces.clear();
	}

	ClientDispatcher &ClientDispatcher::getInstance()
	{
		static ClientDispatcher clientDispatcher;

		return clientDispatcher;
	}

	bool ClientDispatcher::addSEService(void *context, SEService *service)
	{
		bool result = true;
		map<void *, SEService *>::iterator item;

		_BEGIN();

		if ((item = mapSESerivces.find(context)) == mapSESerivces.end())
		{
			mapSESerivces.insert(make_pair(context, service));
		}
		else
		{
			_DBG("SEService [%p] exists", context);
		}

		_END();

		return result;
	}

	void ClientDispatcher::removeSEService(void *context)
	{
		map<void *, SEService *>::iterator item;

		_BEGIN();

		if ((item = mapSESerivces.find(context)) != mapSESerivces.end())
		{
			mapSESerivces.erase(item);
		}
		else
		{
			_DBG("SEService doesn't exist");
		}

		_END();
	}

	void *ClientDispatcher::dispatcherThreadFunc(DispatcherMsg *msg, void *data)
	{
		_BEGIN();

		if (msg == NULL)
			return NULL;

		/* this messages are response from server */
		switch (msg->message)
		{
		/* SE Service requests */
		case Message::MSG_REQUEST_READERS :
		case Message::MSG_REQUEST_SHUTDOWN :
			{
				DispatcherMsg *tempMsg = new DispatcherMsg(msg);

				if (msg->isSynchronousCall() == false)
				{
					/* Asynchronous call */
					g_idle_add((GSourceFunc)&SEService::dispatcherCallback, (gpointer)tempMsg);
				}
				else
				{
					/* Synchronous call */
					SEService::dispatcherCallback(tempMsg);
				}
			}
			break;

		/* Reader requests */
		case Message::MSG_REQUEST_OPEN_SESSION :
			{
				DispatcherMsg *tempMsg = new DispatcherMsg(msg);

				if (msg->isSynchronousCall() == false)
				{
					/* Asynchronous call */
					g_idle_add((GSourceFunc)&Reader::dispatcherCallback, (gpointer)tempMsg);
				}
				else
				{
					/* Synchronous call */
					Reader::dispatcherCallback(tempMsg);
				}
			}
			break;

		/* Session requests */
		case Message::MSG_REQUEST_OPEN_CHANNEL :
		case Message::MSG_REQUEST_GET_ATR :
		case Message::MSG_REQUEST_CLOSE_SESSION :
		case Message::MSG_REQUEST_GET_CHANNEL_COUNT :
			{
				DispatcherMsg *tempMsg = new DispatcherMsg(msg);

				if (msg->isSynchronousCall() == false)
				{
					/* Asynchronous call */
					g_idle_add((GSourceFunc)&Session::dispatcherCallback, (gpointer)tempMsg);
				}
				else
				{
					/* Synchronous call */
					Session::dispatcherCallback(tempMsg);
				}
			}
			break;

		/* ClientChannel requests */
		case Message::MSG_REQUEST_TRANSMIT :
		case Message::MSG_REQUEST_CLOSE_CHANNEL :
			{
				DispatcherMsg *tempMsg = new DispatcherMsg(msg);

				if (msg->isSynchronousCall() == false)
				{
					/* Asynchronous call */
					g_idle_add((GSourceFunc)&ClientChannel::dispatcherCallback, (gpointer)tempMsg);
				}
				else
				{
					/* Synchronous call */
					ClientChannel::dispatcherCallback(tempMsg);
				}
			}
			break;

		case Message::MSG_NOTIFY_SE_INSERTED :
		case Message::MSG_NOTIFY_SE_REMOVED :
			{
				map<void *, SEService *>::iterator item;

				for (item = mapSESerivces.begin(); item != mapSESerivces.end(); item++)
				{
					DispatcherMsg *tempMsg = new DispatcherMsg(msg);

					tempMsg->caller = item->second;

					/* Always asynchronous call */
					g_idle_add((GSourceFunc)&SEService::dispatcherCallback, (gpointer)tempMsg);
				}
			}
			break;

		case Message::MSG_OPERATION_RELEASE_CLIENT :
			{
				map<void *, SEService *>::iterator item;

				for (item = mapSESerivces.begin(); item != mapSESerivces.end(); item++)
				{
					DispatcherMsg *tempMsg = new DispatcherMsg(msg);

					tempMsg->caller = item->second;
					tempMsg->error = -1;

					/* Always asynchronous call */
					g_idle_add((GSourceFunc)&SEService::dispatcherCallback, (gpointer)tempMsg);
				}
			}
			break;

		default :
			break;
		}

		_END();

		return NULL;
	}

} /* namespace open_mobile_api */

