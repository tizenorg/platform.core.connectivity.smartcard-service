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
#include <dirent.h>
#include <dlfcn.h>
#include <errno.h>

/* SLP library header */

/* local header */
#include "Debug.h"
#include "Message.h"
#include "TerminalInterface.h"
#include "ServerSEService.h"
#include "ServerResource.h"

namespace smartcard_service_api
{
#define OMAPI_SE_PATH LIBPATH"/se"

	ServerSEService::ServerSEService():SEServiceHelper()
	{
	}

	ServerSEService::~ServerSEService()
	{
	}

	ServerSEService &ServerSEService::getInstance()
	{
		static ServerSEService seService;

		return seService;
	}

	Terminal *ServerSEService::createInstance(void *library)
	{
		Terminal *terminal = NULL;
		terminal_create_instance_fn createInstance = NULL;

		/* create se instance */
		createInstance = (terminal_create_instance_fn)dlsym(library, "create_instance");
		if (createInstance != NULL)
		{
			terminal = (Terminal *)createInstance();
			if (terminal != NULL)
			{
				SCARD_DEBUG("terminal [%p]", terminal);
			}
			else
			{
				SCARD_DEBUG_ERR("terminal is null");
			}
		}
		else
		{
			SCARD_DEBUG_ERR("create_instance is null [%d]", errno);
		}

		return terminal;
	}

	bool ServerSEService::appendSELibrary(char *library)
	{
		void *libHandle = NULL;
		bool result = false;

		libHandle = dlopen(library, RTLD_LAZY);
		if (libHandle != NULL)
		{
			Terminal *terminal = NULL;

			terminal = createInstance(libHandle);
			if (terminal != NULL)
			{
				SCARD_DEBUG("SE info : [%s] [%s]", library, terminal->getName());

				libraries.push_back(libHandle);

				pair<char *, Terminal *> newPair(terminal->getName(), terminal);
				mapTerminals.insert(newPair);

				if (terminal->isSecureElementPresence() == true)
				{
					ServerReader *reader = new ServerReader(this, terminal->getName(), terminal);
					if (reader != NULL)
					{
						SCARD_DEBUG("register success [%s]", terminal->getName());

						readers.push_back(reader);
					}
					else
					{
						SCARD_DEBUG_ERR("ServerReader alloc failed [%s]", terminal->getName());
						/* throw exception */
					}
				}
				else
				{
					SCARD_DEBUG("SE is not ready [%s]", terminal->getName());
				}

				result = true;
			}
			else
			{
				SCARD_DEBUG_ERR("createInstance failed [%s]", library);

				dlclose(libHandle);
			}
		}
		else
		{
			SCARD_DEBUG_ERR("it is not se file [%s] [%d]", library, errno);
		}

		return result;
	}

	int ServerSEService::openSELibraries()
	{
		int result;
		void *libHandle;
		DIR *dir = NULL;
		struct dirent *entry = NULL;

		if ((dir = opendir(OMAPI_SE_PATH)) != NULL)
		{
			while ((entry = readdir(dir)) != NULL)
			{
				if (strncmp(entry->d_name, ".", 1) != 0 && strncmp(entry->d_name, "..", 2) != 0)
				{
					char fullPath[1024] = { 0, };

					/* need additional name rule :) */
					/* open each files */
					libHandle = NULL;

					snprintf(fullPath, sizeof(fullPath), "%s/%s", OMAPI_SE_PATH, entry->d_name);

					SCARD_DEBUG("se name [%s]", fullPath);

					result = appendSELibrary(fullPath);
				}
			}

			closedir(dir);

			result = 0;
		}
		else
		{
			result = -1;
		}

		return result;
	}

	void ServerSEService::closeSELibraries()
	{
		if (libraries.size() > 0)
		{
			size_t i;

			for (i = 0; i < libraries.size(); i++)
			{
				if (libraries[i] != NULL)
					dlclose(libraries[i]);
			}
		}
	}

	bool ServerSEService::dispatcherCallback(void *message, int socket)
	{
		int count;
		ByteArray info;
		Message *msg = (Message *)message;
		Message response(*msg);
		ServerResource &resource = ServerResource::getInstance();

		if (resource.createService(socket, msg->error) != NULL)
		{
			SCARD_DEBUG_ERR("client added : pid [%d]", msg->error);

			response.error = SCARD_ERROR_OK;

			if ((count = resource.getReadersInformation(info)) > 0)
			{
				response.param1 = count;
				response.data = info;
			}
			else
			{
				SCARD_DEBUG("no secure elements");
				response.param1 = 0;
			}
		}
		else
		{
			SCARD_DEBUG_ERR("createClient failed");

			response.error = SCARD_ERROR_OUT_OF_MEMORY;
		}

		/* response to client */
		ServerIPC::getInstance()->sendMessage(socket, &response);

		return false;
	}

	void ServerSEService::terminalCallback(void *terminal, int event, int error, void *user_param)
	{
		Message msg;
//		Terminal *term = NULL;

		switch (event)
		{
		case Terminal::NOTIFY_SE_AVAILABLE :
			/* send all client to refresh reader */
			msg.message = msg.MSG_NOTIFY_SE_INSERTED;
			msg.data.setBuffer((unsigned char *)terminal,
				strlen((char *)terminal) + 1);

			ServerResource::getInstance().sendMessageToAllClients(msg);
			break;

		case Terminal::NOTIFY_SE_NOT_AVAILABLE :
			/* send all client to refresh reader */
			msg.message = msg.MSG_NOTIFY_SE_REMOVED;
			msg.data.setBuffer((unsigned char *)terminal,
				strlen((char *)terminal) + 1);

			ServerResource::getInstance().sendMessageToAllClients(msg);
			break;

		default :
			break;
		}
	}

} /* namespace smartcard_service_api */
