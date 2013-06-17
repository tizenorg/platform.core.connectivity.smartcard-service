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
#include <dbus/dbus-glib.h>

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

bool net_nfc_is_authorized_nfc_access(const char *package, uint8_t *aid, uint32_t aid_len)
{
	bool result = false;
	DBusGConnection *connection;
	GError *error = NULL;

	dbus_g_thread_init();

	g_type_init();

	connection = dbus_g_bus_get(DBUS_BUS_SYSTEM, &error);
	if (error == NULL)
	{
		DBusGProxy *proxy;

		proxy = dbus_g_proxy_new_for_name(connection, "org.tizen.smartcard_service",
			"/org/tizen/smartcard_service", "org.tizen.smartcard_service");
		if (proxy != NULL)
		{
			gint rule = 0;
			GArray temp = { (gchar *)aid, aid_len };

			if (dbus_g_proxy_call(proxy, "check_nfc_access", &error,
				G_TYPE_STRING, package,
				dbus_g_type_get_collection("GArray", G_TYPE_UCHAR), &temp, G_TYPE_INVALID,
				G_TYPE_INT, rule, G_TYPE_INVALID) == false)
			{
				_ERR("check_nfc_access failed");
				if (error != NULL)
				{
					_ERR("message : [%s]", error->message);
					g_error_free(error);
				}
			}
			else
			{
				_INFO("check_nfc_access returns : %d", rule);
				result = !!rule;
			}
		}
		else
		{
			_ERR("ERROR: Can't make dbus proxy");
		}
	}
	else
	{
		_ERR("ERROR: Can't get on system bus [%s]", error->message);
		g_error_free(error);
	}

	return result;
}

TestEventHandler testEventHandler;

void testConnectedCallback(SEServiceHelper *service, void *userData)
{
	vector<ReaderHelper *> readers;
	user_context_t *context = (user_context_t *)userData;
	uint8_t buffer[] = { 0xA0, 0x00, 0x00, 0x00, 0x63, 0x50, 0x4B, 0x43, 0x53, 0x2D, 0x31, 0x35 };
//	uint8_t buffer[] = { 0xA0, 0x00, 0x00, 0x00, 0x63, 0x50, 0x4B, 0x43, 0x53, 0x2D, 0x31, 0x35 };

	_BEGIN();

//	net_nfc_is_authorized_nfc_access("autoutc099.SecureElementUnitTest", buffer, sizeof(buffer));
	if (service != NULL)
	{
		_DBG("callback called, service [%p]", service);

		context->clientService = service;

		readers = service->getReaders();

		size_t i;
		for (i = 0; i < readers.size(); i++)
		{
			Reader *reader = NULL;

			reader = (Reader *)readers[i];

			_DBG("reader [%p]", reader);

			Session *session = (Session *)reader->openSessionSync();
			if (session != NULL)
			{
				_DBG("session [%p]", session);

				ByteArray temp;
				try
				{
					temp = session->getATRSync();
				}
				catch (...)
				{
					_ERR("exception....");
				}
				_DBG("atr[%d] : %s", temp.getLength(), temp.toString());

				ByteArray aid;

				aid.setBuffer(buffer, sizeof(buffer));
				try
				{
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
				}
				catch (...)
				{
					_ERR("exception....");
				}

				session->closeSync();
			}
			else
			{
				_ERR("openSessionSync failed");
			}
		}

		((SEService *)service)->shutdownSync();
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
