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
#include <string.h>
#include <dlfcn.h>
#include <errno.h>
#include <dirent.h>

/* SLP library header */

/* local header */
#include "Debug.h"
#include "ServerResource.h"
#include "TerminalInterface.h"
#include "APDUHelper.h"
#include "SignatureHelper.h"
#include "GPSEACL.h"

namespace smartcard_service_api
{
	unsigned int IntegerHandle::newHandle = 0;
	set<unsigned int> IntegerHandle::setHandles;
	PMutex IntegerHandle::mutexLock;

	unsigned int IntegerHandle::assignHandle()
	{
		SCOPE_LOCK(mutexLock)
		{
			pair<set<unsigned int>::iterator, bool> result;

			do
			{
				newHandle++;
				if (newHandle == (unsigned int)-1)
				{
					newHandle = 1;
				}

				result = setHandles.insert(newHandle);

			}
			while (!result.second);
		}

		SCARD_DEBUG("assign handle : newHandle [%d]", newHandle);

		return newHandle;
	}

	void IntegerHandle::releaseHandle(unsigned int handle)
	{
		SCARD_DEBUG("will be released : Handle [%d]", handle);

		SCOPE_LOCK(mutexLock)
		{
			setHandles.erase(handle);
		}
	}

#define OMAPI_SE_PATH "/usr/lib/se"

	ServerResource::ServerResource()
	{
		SCARD_BEGIN();

		serverIPC = ServerIPC::getInstance();
		serverDispatcher = ServerDispatcher::getInstance();

#if 1
		loadSecureElements();
#endif
		SCARD_END();
	}

	ServerResource::~ServerResource()
	{
	}

	ServerResource &ServerResource::getInstance()
	{
		static ServerResource serverResource;

		return serverResource;
	}

	bool ServerResource::createClient(void *ioChannel, int socket, int watchID, int state, int pid)
	{
		bool result = false;

		if (getClient(socket) == NULL)
		{
			ClientInstance *instance = new ClientInstance(ioChannel, socket, watchID, state, pid);
			if (instance != NULL)
			{
				mapClients.insert(make_pair(socket, instance));
				result = true;
			}
			else
			{
				SCARD_DEBUG_ERR("alloc failed");
			}
		}
		else
		{
			SCARD_DEBUG_ERR("client already exist [%d]", socket);
		}

		return result;
	}

	ClientInstance *ServerResource::getClient(int socket)
	{
		ClientInstance *result = NULL;
		map<int, ClientInstance *>::iterator item;

		if ((item = mapClients.find(socket)) != mapClients.end())
		{
			result = item->second;
		}

		return result;
	}

	void ServerResource::setPID(int socket, int pid)
	{
		map<int, ClientInstance *>::iterator item;

		if ((item = mapClients.find(socket)) != mapClients.end())
		{
			if (item->second->getPID() < 0)
				item->second->setPID(pid);
		}
	}

	void ServerResource::removeClient(int socket)
	{
		map<int, ClientInstance *>::iterator item;

		if ((item = mapClients.find(socket)) != mapClients.end())
		{
			ServerIPC::getInstance()->releaseClient(item->second->getIOChannel(), item->second->getSocket(), item->second->getWatchID());

			delete item->second;
			mapClients.erase(item);
		}
		else
		{
			SCARD_DEBUG("client exists already [%d]", socket);
		}
	}

	void ServerResource::removeClients()
	{
		map<int, ClientInstance *>::iterator item;

		for (item = mapClients.begin(); item != mapClients.end(); item++)
		{
			ServerIPC::getInstance()->releaseClient(item->second->getIOChannel(), item->second->getSocket(), item->second->getWatchID());

			delete item->second;
		}

		mapClients.clear();
	}

	bool ServerResource::createService(int socket, unsigned int context)
	{
		bool result = false;
		ClientInstance *instance = NULL;

		if ((instance = getClient(socket)) != NULL)
		{
			if ((result = instance->createService(context)) == false)
			{
				SCARD_DEBUG_ERR("ClientInstance::createService failed [%d] [%d]", socket, context);
			}
		}
		else
		{
			SCARD_DEBUG_ERR("client doesn't exist [%d]", socket);
		}

		return result;
	}

	ServiceInstance *ServerResource::getService(int socket, unsigned int context)
	{
		ServiceInstance *result = NULL;
		ClientInstance *instance = NULL;

		if ((instance = getClient(socket)) != NULL)
		{
			result = instance->getService(context);
		}
		else
		{
			SCARD_DEBUG_ERR("client doesn't exist [%d]", socket);
		}

		return result;
	}

	void ServerResource::removeService(int socket, unsigned int context)
	{
		ClientInstance *instance = NULL;

		if ((instance = getClient(socket)) != NULL)
		{
			instance->removeService(context);
		}
		else
		{
			SCARD_DEBUG_ERR("client doesn't exist [%d]", socket);
		}
	}

	void ServerResource::removeServices(int socket)
	{
		ClientInstance *instance = NULL;

		if ((instance = getClient(socket)) != NULL)
		{
			instance->removeServices();
		}
		else
		{
			SCARD_DEBUG_ERR("client doesn't exist [%d]", socket);
		}
	}

	Terminal *ServerResource::getTerminal(unsigned int terminalID)
	{
		Terminal *result = NULL;
		map<unsigned int, Terminal *>::iterator item;

		if ((item = mapTerminals.find(terminalID)) != mapTerminals.end())
		{
			result = item->second;
		}
		else
		{
			SCARD_DEBUG_ERR("Terminal doesn't exist [%d]", terminalID);
		}

		return result;
	}

	Terminal *ServerResource::getTerminal(const char *name)
	{
		Terminal *result = NULL;
		map<unsigned int, Terminal *>::iterator item;

		for (item = mapTerminals.begin(); item != mapTerminals.end(); item++)
		{
			if (strncmp(name, item->second->getName(), strlen(name)) == 0)
			{
				result = item->second;
				break;
			}
		}

		return result;
	}

	unsigned int ServerResource::createSession(int socket, unsigned int context, unsigned int terminalID, ByteArray packageCert, void *caller)
	{
		unsigned int result = -1;
		Terminal *temp = NULL;
		ServiceInstance *instance = NULL;

		if ((instance = getService(socket, context)) != NULL)
		{
			if ((temp = getTerminal(terminalID)) != NULL)
			{
				result = instance->openSession(temp, packageCert, caller);
			}
		}
		else
		{
			SCARD_DEBUG_ERR("getService doesn't exist : socket [%d], context [%d]", socket, context);
		}

		return result;
	}

	ServerSession *ServerResource::getSession(int socket, unsigned int context, unsigned int sessionID)
	{
		ServerSession *result = NULL;
		ServiceInstance *instance = NULL;

		if ((instance = getService(socket, context)) != NULL)
		{
			result = instance->getSession(sessionID);
		}
		else
		{
			SCARD_DEBUG_ERR("Session doesn't exist : socket [%d], context [%d], handle [%d]", socket, context, sessionID);
		}

		return result;
	}

	unsigned int ServerResource::getChannelCount(int socket, unsigned int context, unsigned int sessionID)
	{
		unsigned int result = -1;
		ServiceInstance *instance = NULL;

		if ((instance = getService(socket, context)) != NULL)
		{
			result = instance->getChannelCountBySession(sessionID);
		}
		else
		{
			SCARD_DEBUG_ERR("getService doesn't exist : socket [%d], context [%d]", socket, context);
		}

		return result;
	}

	void ServerResource::removeSession(int socket, unsigned int context, unsigned int sessionID)
	{
		ServiceInstance *instance = NULL;

		if ((instance = getService(socket, context)) != NULL)
		{
			instance->closeSession(sessionID);
		}
		else
		{
			SCARD_DEBUG_ERR("getService doesn't exist : socket [%d], context [%d]", socket, context);
		}
	}

	unsigned int ServerResource::createChannel(int socket, unsigned int context, unsigned int sessionID, int channelType, ByteArray aid)
	{
		unsigned int result = -1;
		ServiceInstance *client = NULL;

		if ((client = getService(socket, context)) != NULL)
		{
			if (client->isVaildSessionHandle(sessionID) == true)
			{
				AccessControlList *acList = NULL;
				ServerSession *session = NULL;
				Terminal *terminal = NULL;

				terminal = client->getTerminal(sessionID);
				session = client->getSession(sessionID);
				if (terminal != NULL && session != NULL)
				{
					int rv = 0;
					int channelNum = 0;
					ByteArray certHash;
					ByteArray selectResponse;
					ByteArray command;
					char filename[1024] = { 0, };

					/* check exceptional case */
					SignatureHelper::getProcessName(client->getParent()->getPID(), filename, sizeof(filename));
					if (strncmp(filename, "ozD3Dw1MZruTDKHWGgYaDib2B2LV4/nfT+8b/g1Vsk8=", sizeof(filename)) != 0)
					{
#if 1
						certHash = session->packageCert;
#else
						certHash = client->getParent()->getCertificationHash();
#endif
						/* request open channel sequence */
						if ((acList = getAccessControlList(terminal)) == NULL)
						{
							SCARD_DEBUG_ERR("acList is null");

							return result;
						}

						if (acList->isAuthorizedAccess(aid, certHash) == false)
						{
							SCARD_DEBUG_ERR("unauthorized access, aid %s, hash %s", aid.toString(), certHash.toString());

							return result;
						}
					}

					if (channelType == 1)
					{
						ByteArray response;

						/* open channel */
						command = APDUHelper::generateAPDU(APDUHelper::COMMAND_OPEN_LOGICAL_CHANNEL, 0, ByteArray::EMPTY);
						rv = terminal->transmitSync(command, response);

						if (rv == 0 && response.getLength() >= 2)
						{
							ResponseHelper resp(response);

							if (resp.getStatus() == 0)
							{
								channelNum = resp.getDataField()[0];

								SCARD_DEBUG("channelNum [%d]", channelNum);
							}
							else
							{
								SCARD_DEBUG_ERR("status word [%d][ 0x%02X 0x%02X ]", resp.getStatus(), response[response.getLength() - 2], response[response.getLength() - 1]);

								return result;
							}
						}
						else
						{
							SCARD_DEBUG_ERR("select apdu is failed, rv [%d], length [%d]", rv, response.getLength());

							return result;
						}
					}

					/* select aid */
					command = APDUHelper::generateAPDU(APDUHelper::COMMAND_SELECT_BY_DF_NAME, channelNum, aid);
					rv = terminal->transmitSync(command, selectResponse);
					if (rv == 0 && selectResponse.getLength() >= 2)
					{
						ResponseHelper resp(selectResponse);

						if (resp.getStatus() == 0)
						{
							result = client->openChannel(sessionID, channelNum);
							if (result != IntegerHandle::INVALID_HANDLE)
							{
								ServerChannel *temp = (ServerChannel *)client->getChannel(result);
								if (temp != NULL)
								{
									/* set select response */
									temp->selectResponse = selectResponse;
								}
								else
								{
									SCARD_DEBUG_ERR("IS IT POSSIBLE??????????????????");
								}
							}
							else
							{
								SCARD_DEBUG_ERR("channel is null.");
							}
						}
						else
						{
							SCARD_DEBUG_ERR("status word [%d][ 0x%02X 0x%02X ]", resp.getStatus(), selectResponse[selectResponse.getLength() - 2], selectResponse[selectResponse.getLength() - 1]);
						}
					}
					else
					{
						SCARD_DEBUG_ERR("select apdu is failed, rv [%d], length [%d]", rv, selectResponse.getLength());
					}

				}
				else
				{
					SCARD_DEBUG_ERR("session is invalid [%d]", sessionID);
				}
			}
			else
			{
				SCARD_DEBUG_ERR("session is invalid [%d]", sessionID);
			}
		}
		else
		{
			SCARD_DEBUG_ERR("getClient is failed [%d] [%d]", socket, context);
		}

		return result;
	}

	Channel *ServerResource::getChannel(int socket, unsigned int context, unsigned int channelID)
	{
		Channel *result = NULL;
		ServiceInstance *instance = NULL;

		if ((instance = getService(socket, context)) != NULL)
		{
			result = instance->getChannel(channelID);
		}
		else
		{
			SCARD_DEBUG_ERR("Channel doesn't exist : socket [%d], context [%d], handle [%d]", socket, context, channelID);
		}

		return result;
	}

	void ServerResource::removeChannel(int socket, unsigned int context, unsigned int channelID)
	{
		ServiceInstance *instance = NULL;

		if ((instance = getService(socket, context)) != NULL)
		{
			instance->closeChannel(channelID);
		}
		else
		{
			SCARD_DEBUG_ERR("getService doesn't exist : socket [%d], context [%d]", socket, context);
		}
	}

	AccessControlList *ServerResource::getAccessControlList(Terminal *terminal)
	{
		AccessControlList *result = NULL;
		map<Terminal *, AccessControlList *>::iterator item;

		if ((item = mapACL.find(terminal)) == mapACL.end())
		{
			ServerChannel *channel = new ServerChannel(NULL, NULL, 0, terminal);
			if (channel != NULL)
			{
				/* load access control */
				result = new GPSEACL(channel);
				if (result != NULL)
				{
					result->loadACL();

					mapACL.insert(make_pair(terminal, result));
				}
				else
				{
					SCARD_DEBUG_ERR("alloc failed");
				}
			}
			else
			{
				SCARD_DEBUG_ERR("alloc failed");
			}
		}
		else
		{
			result = item->second;
		}

		return result;
	}

	Terminal *ServerResource::createInstance(void *library)
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

	bool ServerResource::appendSELibrary(char *library)
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
				unsigned int handle = IntegerHandle::assignHandle();

				mapTerminals.insert(make_pair(handle, terminal));
				libraries.push_back(libHandle);

				terminal->setStatusCallback(&ServerResource::terminalCallback);

				SCARD_DEBUG("register success [%s] [%p] [%s] [%p]", library, libHandle, terminal->getName(), terminal);

				result = true;
			}
			else
			{
				SCARD_DEBUG_ERR("terminal is null [%s]", library);

				dlclose(libHandle);
			}
		}
		else
		{
			SCARD_DEBUG_ERR("it is not se file [%s] [%d]", library, errno);
		}

		return result;
	}

	int ServerResource::loadSecureElements()
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

					/* TODO : need additional name rule :) */

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

	void ServerResource::unloadSecureElements()
	{
		size_t i;
		map<unsigned int, Terminal *>::iterator item;

		for (item = mapTerminals.begin(); item != mapTerminals.end(); item++)
		{
			item->second->finalize();

			IntegerHandle::releaseHandle(item->first);
		}

		mapTerminals.clear();

		for (i = 0; i < libraries.size(); i++)
		{
			if (libraries[i] != NULL)
				dlclose(libraries[i]);
		}

		libraries.clear();
	}

	bool ServerResource::isValidReaderHandle(unsigned int reader)
	{
		return (getTerminal(reader) != NULL);
	}

	bool ServerResource::isValidSessionHandle(int socket, unsigned int context, unsigned int session)
	{
		ServiceInstance *instance = NULL;

		return (((instance = getService(socket, context)) != NULL) && (getService(socket, context)->isVaildSessionHandle(session)));
	}

	int ServerResource::getReadersInformation(ByteArray &info)
	{
		int result = 0;
		unsigned char *buffer = NULL;
		unsigned int length = 0;
		unsigned int offset = 0;
		unsigned int nameLen = 0;

		if (mapTerminals.size() > 0)
		{
			map<unsigned int, Terminal *>::iterator item;

			for (item = mapTerminals.begin(); item != mapTerminals.end(); item++)
			{
				if (item->second->isSecureElementPresence())
				{
					length += sizeof(nameLen) + strlen(item->second->getName()) + sizeof(unsigned int);
					result++;
				}
			}

			buffer = new unsigned char[length];
			if (buffer != NULL)
			{
				memset(buffer, 0, length);

				for (item = mapTerminals.begin(); item != mapTerminals.end(); item++)
				{
					if (item->second->isSecureElementPresence())
					{
						nameLen = strlen(item->second->getName());

						memcpy(buffer + offset, &nameLen, sizeof(nameLen));
						offset += sizeof(nameLen);

						memcpy(buffer + offset, item->second->getName(), nameLen);
						offset += nameLen;

						memcpy(buffer + offset, &item->first, sizeof(unsigned int));
						offset += sizeof(unsigned int);
					}
				}

				info.setBuffer(buffer, length);
				delete []buffer;
			}
			else
			{
				SCARD_DEBUG_ERR("alloc failed");
				result = -1;
			}
		}
		else
		{
			SCARD_DEBUG("no secure element");
		}

		return result;
	}

	bool ServerResource::sendMessageToAllClients(Message &msg)
	{
		bool result = true;
		map<int, ClientInstance *>::iterator item;

		for (item = mapClients.begin(); item != mapClients.end(); item++)
		{
			if (item->second->sendMessageToAllServices(item->second->getSocket(), msg) == false)
				result = false;
		}

		return result;
	}

	void ServerResource::terminalCallback(void *terminal, int event, int error, void *user_param)
	{
		SCARD_BEGIN();

		switch (event)
		{
		case Terminal::NOTIFY_SE_AVAILABLE :
			{
				Message msg;

				SCARD_DEBUG("[NOTIFY_SE_AVAILABLE]");

				SCARD_DEBUG("terminal [%s], event [%d], error [%d], user_param [%p]", (char *)terminal, event, error, user_param);

				/* send all client to refresh reader */
				msg.message = msg.MSG_NOTIFY_SE_INSERTED;
				msg.data.setBuffer((unsigned char *)terminal, strlen((char *)terminal) + 1);

				ServerResource::getInstance().sendMessageToAllClients(msg);
			}
			break;

		case Terminal::NOTIFY_SE_NOT_AVAILABLE :
			{
				Message msg;

				SCARD_DEBUG("[NOTIFY_SE_NOT_AVAILABLE]");

				SCARD_DEBUG("terminal [%s], event [%d], error [%d], user_param [%p]", (char *)terminal, event, error, user_param);

				/* send all client to refresh reader */
				msg.message = msg.MSG_NOTIFY_SE_REMOVED;
				msg.data.setBuffer((unsigned char *)terminal, strlen((char *)terminal) + 1);

				ServerResource::getInstance().sendMessageToAllClients(msg);
			}
			break;

		default :
			break;
		}

		SCARD_END();
	}

} /* namespace smartcard_service_api */
