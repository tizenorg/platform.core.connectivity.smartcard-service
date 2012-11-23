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
		SCARD_BEGIN();
		testConnectedCallback(service, userData);
		SCARD_END();
	}

	void eventHandler(SEServiceHelper *service, char *seName, int event, void *userData)
	{
		SCARD_BEGIN();

		SCARD_DEBUG("event occured service [%p], seName[%p], event [%d]", service, seName, event);

		SCARD_END();
	}

	void errorHandler(SEServiceHelper *service, int error, void *userData)
	{
		SCARD_BEGIN();

		SCARD_DEBUG("error occured service [%p], error [%d]", service, error);

		SCARD_END();
	}
};

TestEventHandler testEventHandler;

void testConnectedCallback(SEServiceHelper *service, void *userData)
{
	vector<ReaderHelper *> readers;
	user_context_t *context = (user_context_t *)userData;

	SCARD_BEGIN();

	if (service != NULL)
	{
		SCARD_DEBUG("callback called, service [%p]", service);

		context->clientService = service;

		readers = service->getReaders();

		if (readers.size() > 0)
		{
			Reader *reader = NULL;

			reader = (Reader *)readers[0];

			SCARD_DEBUG("reader [%p]", reader);

			Session *session = (Session *)reader->openSessionSync();
			if (session != NULL)
			{
				SCARD_DEBUG("session [%p]", session);

				ByteArray temp;
				temp = session->getATRSync();
				SCARD_DEBUG("atr[%d] : %s", temp.getLength(), temp.toString());

				unsigned char MF[] = { 0xA0, 0x00, 0x00, 0x00, 0x63, 0x50, 0x4B, 0x43, 0x53, 0x2D, 0x31, 0x35 };
				ByteArray aid;

				aid.setBuffer(MF, sizeof(MF));
				ClientChannel *channel = (ClientChannel *)session->openLogicalChannelSync(aid);
				if (channel != NULL)
				{
					SCARD_DEBUG("channel [%p]", channel);
					ByteArray response;
					ByteArray data, command;
					int fid = 0x00003150;

					response = channel->getSelectResponse();
					SCARD_DEBUG("response : %s", response.toString());

					SCARD_DEBUG("isBasicChannel() = %s", channel->isBasicChannel() ? "Basic" : "Logical");
					SCARD_DEBUG("isClosed() = %s", channel->isClosed() ? "Closed" : "Opened");

					data.setBuffer((unsigned char *)&fid, 2);
					command = APDUHelper::generateAPDU(APDUHelper::COMMAND_SELECT_BY_ID, 0, data);
					int error = channel->transmitSync(command, response);

					SCARD_DEBUG("error : %d, response : %s", error, response.toString());

					channel->closeSync();
				}
				else
				{
					SCARD_DEBUG_ERR("openLogicalChannelSync failed");
				}

				session->closeSync();
			}
			else
			{
				SCARD_DEBUG_ERR("openSessionSync failed");
			}

			service->shutdown();
		}
		else
		{
			SCARD_DEBUG_ERR("reader is empty");
		}
	}
	else
	{
		SCARD_DEBUG_ERR("service is NULL");
	}

	g_main_loop_quit(loop);

	SCARD_END();
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
