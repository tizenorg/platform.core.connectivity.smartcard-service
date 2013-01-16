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
//		user_context_t *context = (user_context_t *)userData;
		vector<ReaderHelper *> readers;
		size_t i;

		SCARD_BEGIN();

		SCARD_DEBUG("event occured service [%p], seName[%s], event [%d], userData [%p]", service, seName, event, userData);

		readers = service->getReaders();

		for (i = 0; i < readers.size(); i++)
		{
			SCARD_DEBUG("Reader[%d] : name [%s], %s", i, readers[i]->getName(), readers[i]->isSecureElementPresent() ? "available" : "unavailable");
		}

		if (event == 1)
		{
			testConnectedCallback(service, userData);
		}

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

void testCloseCallback(int error, void *userData)
{
	user_context_t *context = (user_context_t *)userData;

	SCARD_DEBUG("result [%d], userData [%p]", error, userData);

	context->clientService->shutdown();
}

void testTransmitCallback(unsigned char *buffer, unsigned int length, int error, void *userData)
{
	ByteArray response(buffer, length);
	user_context_t *context = (user_context_t *)userData;

	SCARD_DEBUG("buffer [%p], length [%d], error [%d], userData [%p]", buffer, length, error, userData);

	context->clientChannel->close(testCloseCallback, userData);
}

void testOpenChannelCallback(Channel *channel, int error, void *userData)
{
	SCARD_DEBUG("channel [%p]", channel);

	if (error == 0 && channel != NULL)
	{
		ByteArray response;
		ByteArray data, command;
		int fid = 0x00003150;
		user_context_t *context = (user_context_t *)userData;

		context->clientChannel = channel;

		response = channel->getSelectResponse();

		SCARD_DEBUG("response : %s", response.toString());

		SCARD_DEBUG("isBasicChannel() = %s", channel->isBasicChannel() ? "Basic" : "Logical");
		SCARD_DEBUG("isClosed() = %s", channel->isClosed() ? "Closed" : "Opened");

		data.setBuffer((unsigned char *)&fid, 2);
		command = APDUHelper::generateAPDU(APDUHelper::COMMAND_SELECT_BY_ID, 0, data);
		context->clientChannel->transmit(command, testTransmitCallback, userData);
	}
	else
	{
		SCARD_DEBUG_ERR("openBasicChannel failed");
	}
}

void testGetATRCallback(unsigned char *atr, unsigned int length, int error, void *userData)
{
//	unsigned char MF[] = { 0x3F, 0x00 };
	unsigned char MF[] = { 0xA0, 0x00, 0x00, 0x00, 0x63, 0x50, 0x4B, 0x43, 0x53, 0x2D, 0x31, 0x35 };
	ByteArray aid, result(atr, length);
	user_context_t *context = (user_context_t *)userData;

	SCARD_DEBUG("atr[%d] : %s", result.getLength(), result.toString());

	aid.setBuffer(MF, sizeof(MF));
	context->clientSession->openLogicalChannel(aid, testOpenChannelCallback, userData);
}

void testCloseSessionCallback(int error, void *userData)
{

}

void testOpenSessionCallback(SessionHelper *session, int error, void *userData)
{
	SCARD_DEBUG("session [%p]", session);

	if (session != NULL)
	{
		user_context_t *context = (user_context_t *)userData;

		context->clientSession = (Session *)session;
		context->clientSession->getATR(testGetATRCallback, userData);
	}
	else
	{
		SCARD_DEBUG_ERR("openSession failed");
	}
}

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

			reader->openSession(testOpenSessionCallback, userData);
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
