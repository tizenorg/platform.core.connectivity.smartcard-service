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
#include "TerminalInterface.h"
#include "ServerResource.h"
#include "ServerSEService.h"

namespace smartcard_service_api
{
#define OMAPI_SE_PATH "/usr/lib/se"

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
				_DBG("terminal [%p]", terminal);
			}
			else
			{
				_ERR("terminal is null");
			}
		}
		else
		{
			_ERR("create_instance is null [%d]", errno);
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
				_DBG("SE info : [%s] [%s]", library, terminal->getName());

				libraries.push_back(libHandle);

				pair<string, Terminal *> newPair(terminal->getName(), terminal);
				mapTerminals.insert(newPair);

				if (terminal->isSecureElementPresence() == true)
				{
					ServerReader *reader = new ServerReader(this, terminal->getName(), terminal);
					if (reader != NULL)
					{
						_DBG("register success [%s]", terminal->getName());

						readers.push_back(reader);
					}
					else
					{
						_ERR("ServerReader alloc failed [%s]", terminal->getName());
						/* throw exception */
					}
				}
				else
				{
					_DBG("SE is not ready [%s]", terminal->getName());
				}

				result = true;
			}
			else
			{
				_ERR("createInstance failed [%s]", library);

				dlclose(libHandle);
			}
		}
		else
		{
			_ERR("it is not se file [%s] [%d]", library, errno);
		}

		return result;
	}

	int ServerSEService::openSELibraries()
	{
		int result;
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

					snprintf(fullPath, sizeof(fullPath), "%s/%s", OMAPI_SE_PATH, entry->d_name);

					SECURE_LOGD("se name [%s]", fullPath);

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

#if 0
	bool ServerSEService::isValidReaderHandle(void *handle)
	{
		bool result = false;
		size_t i;

		for (i = 0; i < readers.size(); i++)
		{
			if ((void *)readers[i] == handle)
			{
				result =  true;
				break;
			}
		}

		return false;
	}
#endif

	void ServerSEService::terminalCallback(const void *terminal, int event, int error, void *user_param)
	{
		switch (event)
		{
		case Terminal::NOTIFY_SE_AVAILABLE :
			{
				/* add right se reader */
//				if ((term = ServerResource::getInstance().getTerminal((char *)terminal)) != NULL)
//				{
//					_DBG("terminal : [%s]", (char *)terminal);
//
//					term->initialize();
//				}
//				else
//				{
//					_DBG("unknown terminal : [%s]", (char *)terminal);
//				}
			}
			break;

		case Terminal::NOTIFY_SE_NOT_AVAILABLE :
			{
				/* remove right se reader */
//				if ((term = ServerResource::getInstance().getTerminal((char *)terminal)) != NULL)
//				{
//					_DBG("terminal : [%s]", (char *)terminal);
//
//					term->finalize();
//				}
//				else
//				{
//					_DBG("unknown terminal : [%s]", (char *)terminal);
//				}
			}
			break;

		default :
			break;
		}
	}

} /* namespace smartcard_service_api */
