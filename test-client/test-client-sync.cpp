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

#include <stdio.h>
#include <string.h>
#include <glib.h>

#include "Debug.h"
#include "SEService.h"
#include "Reader.h"
#include "Session.h"
#include "APDUHelper.h"
#include "ClientChannel.h"

using namespace smartcard_service_api;

typedef struct _user_context_t
{
	Session *clientSession;
	SEServiceHelper *clientService;
	Channel *clientChannel;
}
user_context_t;

/* global variable */
GMainLoop *loop = NULL;
user_context_t user_context = { 0, };

void testCloseCallback(int error, void *userData);
void testTransmitCallback(unsigned char *buffer, unsigned int length, int error, void *userData);
void testOpenChannelCallback(Channel *channel, int error, void *userData);
void testGetATRCallback(unsigned char *atr, unsigned int length, int error, void *userData);
void testCloseSessionCallback(int error, void *userData);
void testOpenSessionCallback(SessionHelper *session, int error, void *userData);
void testConnectedCallback(SEServiceHelper *service, void *context);

class TestEventHandler : public SEServiceListener
{
	void serviceConnected(SEServiceHelper *service, void *userData)
	{
		_BEGIN();
		testConnectedCallback(service, userData);
		_END();
	}

	void eventHandler(SEServiceHelper *service, char *seName, int event, void *userData)
	{
		_BEGIN();

		_INFO("event occurred service [%p], seName[%p], event [%d]", service, seName, event);

		_END();
	}

	void errorHandler(SEServiceHelper *service, int error, void *userData)
	{
		_BEGIN();

		_ERR("error occurred service [%p], error [%d]", service, error);

		_END();
	}
};

TestEventHandler testEventHandler;

void testConnectedCallback(SEServiceHelper *service, void *userData)
{
	vector<ReaderHelper *> readers;
	user_context_t *context = (user_context_t *)userData;

	_BEGIN();

	if (service != NULL)
	{
		_DBG("callback called, service [%p]", service);

		context->clientService = service;

		readers = service->getReaders();

		if (readers.size() > 0)
		{
			Reader *reader = NULL;

			reader = (Reader *)readers[0];

			_DBG("reader [%p]", reader);

			Session *session = (Session *)reader->openSessionSync();
			if (session != NULL)
			{
				_DBG("session [%p]", session);

				ByteArray temp;
				temp = session->getATRSync();
				_DBG("atr[%d] : %s", temp.getLength(), temp.toString());

				unsigned char MF[] = { 0xA0, 0x00, 0x00, 0x00, 0x63, 0x50, 0x4B, 0x43, 0x53, 0x2D, 0x31, 0x35 };
				ByteArray aid;

				aid.setBuffer(MF, sizeof(MF));
				ClientChannel *channel = (ClientChannel *)session->openLogicalChannelSync(aid);
				if (channel != NULL)
				{
					_DBG("channel [%p]", channel);
					ByteArray response;
					ByteArray data, command;
					int fid = 0x00003150;

					response = channel->getSelectResponse();
					_INFO("response : %s", response.toString());

					_DBG("isBasicChannel() = %s", channel->isBasicChannel() ? "Basic" : "Logical");
					_DBG("isClosed() = %s", channel->isClosed() ? "Closed" : "Opened");

					data.setBuffer((unsigned char *)&fid, 2);
					command = APDUHelper::generateAPDU(APDUHelper::COMMAND_SELECT_BY_ID, 0, data);
					int error = channel->transmitSync(command, response);

					_INFO("error : %d, response : %s", error, response.toString());

					channel->closeSync();
				}
				else
				{
					_ERR("openLogicalChannelSync failed");
				}

				session->closeSync();
			}
			else
			{
				_ERR("openSessionSync failed");
			}

			service->shutdown();
		}
		else
		{
			_ERR("reader is empty");
		}
	}
	else
	{
		_ERR("service is NULL");
	}

	g_main_loop_quit(loop);

	_END();
}

int main(int argv, char *args[])
{
	SEService *service = new SEService((void *)&user_context, &testEventHandler);

	loop = g_main_new(TRUE);
	g_main_loop_run(loop);

	if (service != NULL)
		delete service;

	return 0;
}
