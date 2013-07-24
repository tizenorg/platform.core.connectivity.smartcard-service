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
#include "GPACE.h"
#include "PKCS15.h"
#ifdef USE_GDBUS
#include "ServerGDBus.h"
#endif

#ifndef EXTERN_API
#define EXTERN_API __attribute__((visibility("default")))
#endif

using namespace std;

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

		_DBG("assign handle : newHandle [%d]", newHandle);

		return newHandle;
	}

	void IntegerHandle::releaseHandle(unsigned int handle)
	{
		_DBG("will be released : Handle [%d]", handle);

		SCOPE_LOCK(mutexLock)
		{
			setHandles.erase(handle);
		}
	}

#define OMAPI_SE_PATH "/usr/lib/se"

	ServerResource::ServerResource()
		: mainLoop(NULL), seLoaded(false)
	{
		_BEGIN();
#ifndef USE_GDBUS
		serverIPC = ServerIPC::getInstance();
		serverDispatcher = ServerDispatcher::getInstance();
#endif
		_END();
	}

	ServerResource::~ServerResource()
	{
	}

	ServerResource &ServerResource::getInstance()
	{
		static ServerResource serverResource;

		return serverResource;
	}

#ifdef USE_GDBUS
	bool ServerResource::createClient(const char *name, pid_t pid)
	{
		bool result = false;

		if (getClient(name) == NULL)
		{
			ClientInstance *instance = new ClientInstance(name, pid);
			if (instance != NULL)
			{
				mapClients.insert(make_pair(name, instance));
				result = true;
			}
			else
			{
				_ERR("alloc failed");
			}
		}
		else
		{
			_ERR("client already exist, name [%s]", name);
		}

		return result;
	}

	ClientInstance *ServerResource::getClient(const char *name)
	{
		ClientInstance *result = NULL;
		map<string, ClientInstance *>::iterator item;

		if ((item = mapClients.find(name)) != mapClients.end())
		{
			result = item->second;
		}

		return result;
	}

	void ServerResource::removeClient(const char *name)
	{
		map<string, ClientInstance *>::iterator item;

		if ((item = mapClients.find(name)) != mapClients.end())
		{
			delete item->second;
			mapClients.erase(item);
		}
		else
		{
			_DBG("client removed already, name [%s]", name);
		}
	}

	void ServerResource::removeClients()
	{
		map<string, ClientInstance *>::iterator item;

		for (item = mapClients.begin(); item != mapClients.end(); item++)
		{
			delete item->second;
		}

		mapClients.clear();
	}

	int ServerResource::getClientCount() const
	{
		return (int)mapClients.size();
	}

	ServiceInstance *ServerResource::createService(const char *name)
	{
		ServiceInstance *result = NULL;
		ClientInstance *instance = NULL;

		if ((instance = getClient(name)) != NULL)
		{
			if ((result = instance->createService()) == NULL)
			{
				_ERR("ClientInstance::createService failed [%d]", name);
			}
		}
		else
		{
			_ERR("client doesn't exist, name [%s]", name);
		}

		return result;
	}

	ServiceInstance *ServerResource::getService(const char *name, unsigned int handle)
	{
		ServiceInstance *result = NULL;
		ClientInstance *instance = NULL;

		if ((instance = getClient(name)) != NULL)
		{
			result = instance->getService(handle);
		}
		else
		{
			_ERR("client doesn't exist, name [%s]", name);
		}

		return result;
	}

	void ServerResource::removeService(const char *name, unsigned int handle)
	{
		ClientInstance *instance = NULL;

		if ((instance = getClient(name)) != NULL)
		{
			instance->removeService(handle);
		}
		else
		{
			_ERR("client doesn't exist, name [%s]", name);
		}
	}

	void ServerResource::removeServices(const char *name)
	{
		ClientInstance *instance = NULL;

		if ((instance = getClient(name)) != NULL)
		{
			instance->removeServices();
		}
		else
		{
			_ERR("client doesn't exist, name [%s]", name);
		}
	}

	unsigned int ServerResource::createSession(const char *name, unsigned int handle, unsigned int readerID, vector<ByteArray> &certHashes, void *caller)
	{
		unsigned int result = -1;
		Terminal *temp = NULL;
		ServiceInstance *instance = NULL;

		if ((instance = getService(name, handle)) != NULL)
		{
			if ((temp = getTerminalByReaderID(readerID)) != NULL)
			{
				result = instance->openSession(temp, certHashes, caller);
			}
		}
		else
		{
			_ERR("getService doesn't exist : name [%s], handle [%d]", name, handle);
		}

		return result;
	}

	ServerSession *ServerResource::getSession(const char *name, unsigned int handle, unsigned int sessionID)
	{
		ServerSession *result = NULL;
		ServiceInstance *instance = NULL;

		if ((instance = getService(name, handle)) != NULL)
		{
			result = instance->getSession(sessionID);
		}
		else
		{
			_ERR("Session doesn't exist : name [%s], handle [%d], handle [%d]", name, handle, sessionID);
		}

		return result;
	}

	bool ServerResource::isValidSessionHandle(const char *name, unsigned int handle, unsigned int session)
	{
		ServiceInstance *instance = NULL;

		return (((instance = getService(name, handle)) != NULL) && (instance->isVaildSessionHandle(session)));
	}

	unsigned int ServerResource::getChannelCount(const char *name, unsigned int handle, unsigned int sessionID)
	{
		unsigned int result = -1;
		ServiceInstance *instance = NULL;

		if ((instance = getService(name, handle)) != NULL)
		{
			result = instance->getChannelCountBySession(sessionID);
		}
		else
		{
			_ERR("getService doesn't exist : name [%s], handle [%d]", name, handle);
		}

		return result;
	}

	void ServerResource::removeSession(const char *name, unsigned int handle, unsigned int sessionID)
	{
		ServiceInstance *instance = NULL;

		if ((instance = getService(name, handle)) != NULL)
		{
			instance->closeSession(sessionID);
		}
		else
		{
			_ERR("getService doesn't exist : name [%s], handle [%d]", name, handle);
		}
	}

	unsigned int ServerResource::createChannel(const char *name, unsigned int handle, unsigned int sessionID, int channelType, ByteArray aid)
		throw(ExceptionBase &)
	{
		unsigned int result = -1;
		ServiceInstance *service = NULL;

		if ((service = getService(name, handle)) != NULL)
		{
			if (service->isVaildSessionHandle(sessionID) == true)
			{
				ServerSession *session = NULL;
				Terminal *terminal = NULL;

				terminal = service->getTerminal(sessionID);
				session = service->getSession(sessionID);
				if (terminal != NULL && session != NULL)
				{
					result = _createChannel(terminal, service, channelType, sessionID, aid);
					if (result == IntegerHandle::INVALID_HANDLE)
					{
						_ERR("create channel failed [%d]", sessionID);
					}
				}
				else
				{
					_ERR("session is invalid [%d]", sessionID);
					throw ExceptionBase(SCARD_ERROR_UNAVAILABLE);
				}
			}
			else
			{
				_ERR("session is invalid [%d]", sessionID);
				throw ExceptionBase(SCARD_ERROR_ILLEGAL_STATE);
			}
		}
		else
		{
			_ERR("getService is failed, name [%s], handle [%d]", name, handle);
			throw ExceptionBase(SCARD_ERROR_UNAVAILABLE);
		}

		return result;
	}

	Channel *ServerResource::getChannel(const char *name, unsigned int handle, unsigned int channelID)
	{
		Channel *result = NULL;
		ServiceInstance *instance = NULL;

		if ((instance = getService(name, handle)) != NULL)
		{
			result = instance->getChannel(channelID);
		}
		else
		{
			_ERR("Channel doesn't exist : name [%s], handle [%d], handle [%d]", name, handle, channelID);
		}

		return result;
	}

	void ServerResource::removeChannel(const char *name, unsigned int handle, unsigned int channelID)
	{
		ServiceInstance *instance = NULL;

		if ((instance = getService(name, handle)) != NULL)
		{
			instance->closeChannel(channelID);
		}
		else
		{
			_ERR("getService doesn't exist : name [%s], handle [%d]", name, handle);
		}
	}
#else
	bool ServerResource::createClient(void *ioChannel, int socket,
		int watchID, int state, int pid)
	{
		bool result = false;

		if (getClient(socket) == NULL)
		{
			ClientInstance *instance = new ClientInstance(ioChannel,
				socket, watchID, state, pid);
			if (instance != NULL)
			{
				mapClients.insert(make_pair(socket, instance));
				result = true;
			}
			else
			{
				_ERR("alloc failed");
			}
		}
		else
		{
			_ERR("client already exist, socket[%d]", socket);
		}

		return result;
	}

	bool ServerResource::createClient(int pid)
	{
		bool result = false;

		if (getClient(pid) == NULL)
		{
			ClientInstance *instance = new ClientInstance(pid);
			if (instance != NULL)
			{
				mapClients.insert(make_pair(pid, instance));
				result = true;
			}
			else
			{
				_ERR("alloc failed");
			}
		}
		else
		{
			_ERR("client already exist, pid[%d]", pid);
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

	const ClientInstance *ServerResource::getClient(int socket) const
	{
		const ClientInstance *result = NULL;
		map<int, ClientInstance *>::const_iterator item;

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
#ifndef USE_GDBUS
			ServerIPC::getInstance()->releaseClient(item->second->getIOChannel(), item->second->getSocket(), item->second->getWatchID());
#endif
			delete item->second;
			mapClients.erase(item);
		}
		else
		{
			_DBG("client removed already [%d]", socket);
		}
	}

	void ServerResource::removeClients()
	{
		map<int, ClientInstance *>::iterator item;

		for (item = mapClients.begin(); item != mapClients.end(); item++)
		{
#ifndef USE_GDBUS
			ServerIPC::getInstance()->releaseClient(item->second->getIOChannel(), item->second->getSocket(), item->second->getWatchID());
#endif
			delete item->second;
		}

		mapClients.clear();
	}

	int ServerResource::getClientCount() const
	{
		return (int)mapClients.size();
	}

	ServiceInstance *ServerResource::createService(int socket)
	{
		ServiceInstance *result = NULL;
		ClientInstance *instance = NULL;

		if ((instance = getClient(socket)) != NULL)
		{
			if ((result = instance->createService()) == NULL)
			{
				_ERR("ClientInstance::createService failed [%d]", socket);
			}
		}
		else
		{
			_ERR("client doesn't exist [%d]", socket);
		}

		return result;
	}

	ServiceInstance *ServerResource::getService(int socket, unsigned int handle)
	{
		ServiceInstance *result = NULL;
		ClientInstance *instance = NULL;

		if ((instance = getClient(socket)) != NULL)
		{
			result = instance->getService(handle);
		}
		else
		{
			_ERR("client doesn't exist [%d]", socket);
		}

		return result;
	}

	void ServerResource::removeService(int socket, unsigned int handle)
	{
		ClientInstance *instance = NULL;

		if ((instance = getClient(socket)) != NULL)
		{
			instance->removeService(handle);
		}
		else
		{
			_ERR("client doesn't exist [%d]", socket);
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
			_ERR("client doesn't exist [%d]", socket);
		}
	}

	unsigned int ServerResource::createSession(int socket, unsigned int handle, unsigned int readerID, const vector<ByteArray> &certHashes, void *caller)
	{
		unsigned int result = -1;
		Terminal *temp = NULL;
		ServiceInstance *instance = NULL;

		if ((instance = getService(socket, handle)) != NULL)
		{
			if ((temp = getTerminalByReaderID(readerID)) != NULL)
			{
				result = instance->openSession(temp, certHashes, caller);
			}
		}
		else
		{
			_ERR("getService doesn't exist : socket [%d], handle [%d]", socket, handle);
		}

		return result;
	}

	ServerSession *ServerResource::getSession(int socket, unsigned int handle, unsigned int sessionID)
	{
		ServerSession *result = NULL;
		ServiceInstance *instance = NULL;

		if ((instance = getService(socket, handle)) != NULL)
		{
			result = instance->getSession(sessionID);
		}
		else
		{
			_ERR("Session doesn't exist : socket [%d], handle [%d], handle [%d]", socket, handle, sessionID);
		}

		return result;
	}

	bool ServerResource::isValidSessionHandle(int socket, unsigned int handle, unsigned int session)
	{
		ServiceInstance *instance = NULL;

		return (((instance = getService(socket, handle)) != NULL) && (instance->isVaildSessionHandle(session)));
	}

	unsigned int ServerResource::getChannelCount(int socket, unsigned int handle, unsigned int sessionID)
	{
		unsigned int result = -1;
		ServiceInstance *instance = NULL;

		if ((instance = getService(socket, handle)) != NULL)
		{
			result = instance->getChannelCountBySession(sessionID);
		}
		else
		{
			_ERR("getService doesn't exist : socket [%d], handle [%d]", socket, handle);
		}

		return result;
	}

	void ServerResource::removeSession(int socket, unsigned int handle, unsigned int sessionID)
	{
		ServiceInstance *instance = NULL;

		if ((instance = getService(socket, handle)) != NULL)
		{
			instance->closeSession(sessionID);
		}
		else
		{
			_ERR("getService doesn't exist : socket [%d], handle [%d]", socket, handle);
		}
	}

	unsigned int ServerResource::createChannel(int socket, unsigned int handle, unsigned int sessionID, int channelType, const ByteArray &aid)
		throw(ExceptionBase &)
	{
		unsigned int result = -1;
		ServiceInstance *service = NULL;

		if ((service = getService(socket, handle)) != NULL)
		{
			if (service->isVaildSessionHandle(sessionID) == true)
			{
				ServerSession *session = NULL;
				Terminal *terminal = NULL;

				terminal = service->getTerminal(sessionID);
				session = service->getSession(sessionID);
				if (terminal != NULL && session != NULL)
				{
					result = _createChannel(terminal, service, channelType, sessionID, aid);
					if (result == IntegerHandle::INVALID_HANDLE)
					{
						_ERR("create channel failed [%d]", sessionID);
					}
				}
				else
				{
					_ERR("session is invalid [%d]", sessionID);
					throw ExceptionBase(SCARD_ERROR_UNAVAILABLE);
				}
			}
			else
			{
				_ERR("session is invalid [%d]", sessionID);
				throw ExceptionBase(SCARD_ERROR_ILLEGAL_STATE);
			}
		}
		else
		{
			_ERR("getService is failed [%d] [%d]", socket, handle);
			throw ExceptionBase(SCARD_ERROR_UNAVAILABLE);
		}

		return result;
	}

	Channel *ServerResource::getChannel(int socket, unsigned int handle, unsigned int channelID)
	{
		Channel *result = NULL;
		ServiceInstance *instance = NULL;

		if ((instance = getService(socket, handle)) != NULL)
		{
			result = instance->getChannel(channelID);
		}
		else
		{
			_ERR("Channel doesn't exist : socket [%d], handle [%d], handle [%d]", socket, handle, channelID);
		}

		return result;
	}

	void ServerResource::removeChannel(int socket, unsigned int handle, unsigned int channelID)
	{
		ServiceInstance *instance = NULL;

		if ((instance = getService(socket, handle)) != NULL)
		{
			instance->closeChannel(channelID);
		}
		else
		{
			_ERR("getService doesn't exist : socket [%d], handle [%d]", socket, handle);
		}
	}
#endif
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
			_ERR("Terminal doesn't exist [%d]", terminalID);
		}

		return result;
	}

	const Terminal *ServerResource::getTerminal(unsigned int terminalID) const
	{
		const Terminal *result = NULL;
		map<unsigned int, Terminal *>::const_iterator item;

		if ((item = mapTerminals.find(terminalID)) != mapTerminals.end())
		{
			result = item->second;
		}
		else
		{
			_ERR("Terminal doesn't exist [%d]", terminalID);
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

	Terminal *ServerResource::getTerminalByReaderID(unsigned int readerID)
	{
		Terminal *result = NULL;
		map<unsigned int, unsigned int>::iterator item;

		if ((item = mapReaders.find(readerID)) != mapReaders.end())
		{
			result = getTerminal(item->second);
		}
		else
		{
			_ERR("Terminal doesn't exist, reader ID [%d]", readerID);
		}

		return result;
	}

	const Terminal *ServerResource::getTerminalByReaderID(unsigned int readerID) const
	{
		const Terminal *result = NULL;
		map<unsigned int, unsigned int>::const_iterator item;

		if ((item = mapReaders.find(readerID)) != mapReaders.end())
		{
			result = getTerminal(item->second);
		}
		else
		{
			_ERR("Terminal doesn't exist, reader ID [%d]", readerID);
		}

		return result;
	}

	unsigned int ServerResource::getTerminalID(const char *name) const
	{
		unsigned int result = IntegerHandle::INVALID_HANDLE;
		map<unsigned int, Terminal *>::const_iterator item;

		for (item = mapTerminals.begin();
			item != mapTerminals.end(); item++)
		{
			if (strncmp(name, item->second->getName(),
				strlen(name)) == 0)
			{
				result = item->first;
				break;
			}
		}

		return result;
	}

	bool ServerResource::_isAuthorizedAccess(ServerChannel *channel,
		const ByteArray aid, const vector<ByteArray> &hashes)
	{
		bool result = true;
		AccessControlList *acList = NULL;

		/* request open channel sequence */
		if ((acList = getAccessControlList(channel)) == NULL)
		{
			/* load access control defined by Global Platform */
			GPACE *acl = new GPACE();
			if (acl != NULL)
			{
				int ret;

				ret = acl->loadACL(channel);
				if (ret >= SCARD_ERROR_OK)
				{
					acList = acl;
					addAccessControlList(channel, acList);
				}
				else
				{
					_ERR("unknown error, 0x%x", -ret);
					delete acl;
				}
			}
			else
			{
				_ERR("alloc failed");
			}
		}
		else
		{
			acList->loadACL(channel);
		}

		if (acList != NULL)
		{
			result = acList->isAuthorizedAccess(aid, hashes);
		}

		return result;
	}

	int ServerResource::_openLogicalChannel(Terminal *terminal)
	{
		int result = -1;
		int rv = 0;
		ByteArray command;
		ByteArray response;

		/* open channel */
		command = APDUHelper::generateAPDU(APDUHelper::COMMAND_OPEN_LOGICAL_CHANNEL, 0, ByteArray::EMPTY);
		rv = terminal->transmitSync(command, response);
		if (rv == 0 && response.size() >= 2)
		{
			ResponseHelper resp(response);

			if (resp.getStatus() >= 0)
			{
				result = resp.getDataField()[0];
			}
			else
			{
				result = resp.getStatus();
			}
		}
		else
		{
			_ERR("transmitSync failed, rv [%d], length [%d]", rv, response.size());
		}

		return result;
	}

	int ServerResource::_closeLogicalChannel(Terminal *terminal, int channelNum)
	{
		int result = SCARD_ERROR_UNKNOWN;
		int rv = 0;
		ByteArray command;
		ByteArray response;

		/* open channel */
		command = APDUHelper::generateAPDU(APDUHelper::COMMAND_CLOSE_LOGICAL_CHANNEL, channelNum, ByteArray::EMPTY);
		rv = terminal->transmitSync(command, response);
		if (rv == 0 && response.size() >= 2)
		{
			ResponseHelper resp(response);

			if (resp.getStatus() >= 0)
			{
				_DBG("channel closed [%d]", channelNum);
				result = SCARD_ERROR_OK;
			}
			else
			{
				_ERR("status word [ %02X %02X ]", resp.getSW1(), resp.getSW2());
			}
		}
		else
		{
			_ERR("select apdu is failed, rv [%d], length [%d]", rv, response.size());
		}

		return result;
	}

	unsigned int ServerResource::_createChannel(Terminal *terminal,
		ServiceInstance *service, int channelType,
		unsigned int sessionID, const ByteArray &aid)
		throw(ExceptionBase &)
	{
		unsigned int result = IntegerHandle::INVALID_HANDLE;
		int channelNum = 0;
		ServerChannel *channel = NULL;

		/* open logical channel */
		if (channelType == 1)
		{
			channelNum = _openLogicalChannel(terminal);
			if (channelNum > 0)
			{
				_DBG("channelNum [%d]", channelNum);
			}
			else
			{
				_ERR("_openLogicalChannel failed [%d]", channelNum);
				throw ExceptionBase(SCARD_ERROR_NOT_ENOUGH_RESOURCE);
			}
		}

		/* create channel instance */
		result = service->openChannel(sessionID, channelNum, ByteArray::EMPTY);
		if (result == IntegerHandle::INVALID_HANDLE)
		{
			_ERR("channel is null.");

			/* close logical channel */
			if (channelNum > 0)
			{
				_closeLogicalChannel(terminal, channelNum);
			}
			throw ExceptionBase(SCARD_ERROR_OUT_OF_MEMORY);
		}

		channel = service->getChannel(result);

		/* check */
		if (_isAuthorizedAccess(channel, aid,
			service->getParent()->getCertificationHashes()) == true)
		{
			int rv = 0;

			/* select aid */
			if (aid == PKCS15::PKCS15_AID)
			{
				PKCS15 pkcs15(channel);

				rv = pkcs15.select();
				if (rv >= SCARD_ERROR_OK)
				{
					/* remove privilege mode */
					channel->unsetPrivilegeMode();
					channel->setSelectResponse(pkcs15.getSelectResponse());
				}
				else
				{
					_ERR("select failed, [%x]", -rv);

					service->closeChannel(result);
					throw ExceptionBase(SCARD_ERROR_IO_FAILED);
				}
			}
			else
			{
				FileObject file(channel);

				rv = file.select(aid);
				if (rv >= SCARD_ERROR_OK)
				{
					/* remove privilege mode */
					channel->unsetPrivilegeMode();
					channel->setSelectResponse(file.getSelectResponse());
				}
				else
				{
					_ERR("select failed [%x]", -rv);

					service->closeChannel(result);
					throw ExceptionBase(SCARD_ERROR_IO_FAILED);
				}
			}
		}
		else
		{
			_ERR("unauthorized access");

			service->closeChannel(result);
			throw ExceptionBase(SCARD_ERROR_SECURITY_NOT_ALLOWED);
		}

		return result;
	}

	void ServerResource::addAccessControlList(Terminal *terminal, AccessControlList *acl)
	{
		map<Terminal *, AccessControlList *>::iterator item;

		if ((item = mapACL.find(terminal)) == mapACL.end())
		{
			mapACL.insert(make_pair(terminal, acl));
		}
		else
		{
			item->second = acl;
		}
	}

	void ServerResource::addAccessControlList(ServerChannel *channel, AccessControlList *acl)
	{
		map<Terminal *, AccessControlList *>::iterator item;

		if ((item = mapACL.find(channel->getTerminal())) == mapACL.end())
		{
			mapACL.insert(make_pair(channel->getTerminal(), acl));
		}
		else
		{
			item->second = acl;
		}
	}

	AccessControlList *ServerResource::getAccessControlList(Terminal *terminal)
	{
		AccessControlList *result = NULL;
		map<Terminal *, AccessControlList *>::iterator item;

		if ((item = mapACL.find(terminal)) != mapACL.end())
		{
			result = item->second;
		}

		return result;
	}

	AccessControlList *ServerResource::getAccessControlList(ServerChannel *channel)
	{
		AccessControlList *result = NULL;
		map<Terminal *, AccessControlList *>::iterator item;

		if ((item = mapACL.find(channel->getTerminal())) != mapACL.end())
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

				_DBG("register success [%s] [%p] [%s] [%p]", library, libHandle, terminal->getName(), terminal);

				if (terminal->isSecureElementPresence() == true)
				{
					createReader(handle);
				}

				result = true;
			}
			else
			{
				_ERR("terminal is null [%s]", library);

				dlclose(libHandle);
			}
		}
		else
		{
			_ERR("it is not se file [%s] [%d]", library, errno);
		}

		return result;
	}

	int ServerResource::loadSecureElements()
	{
		int result = 0;

		if (seLoaded == false)
		{
			DIR *dir;
			struct dirent *entry;

			if ((dir = opendir(OMAPI_SE_PATH)) != NULL)
			{
				while ((entry = readdir(dir)) != NULL)
				{
					if (strncmp(entry->d_name, ".", 1) != 0 &&
						strncmp(entry->d_name, "..", 2) != 0)
					{
						char fullPath[1024];

						/* TODO : need additional name rule :) */

						/* append each files */
						snprintf(fullPath, sizeof(fullPath),
							"%s/%s", OMAPI_SE_PATH, entry->d_name);

						SCARD_DEBUG("se name [%s]", fullPath);

						result = appendSELibrary(fullPath);
					}
				}

				closedir(dir);

				seLoaded = true;
			}
			else
			{
				result = -1;
			}
		}

		return result;
	}

	void ServerResource::unloadSecureElements()
	{
		if (seLoaded == true)
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

			seLoaded = false;
		}
	}

	bool ServerResource::isValidReaderHandle(unsigned int reader) const
	{
		return (getTerminalByReaderID(reader) != NULL);
	}

	void ServerResource::getReaders(vector<pair<unsigned int, string> > &readers) const
	{
		const Terminal *terminal;
		map<unsigned int, unsigned int>::const_iterator item;

		readers.clear();

		for (item = mapReaders.begin(); item != mapReaders.end(); item++)
		{
			if (item->second != IntegerHandle::INVALID_HANDLE)
			{
				terminal = getTerminal(item->second);
				if (terminal != NULL && terminal->isSecureElementPresence() == true)
				{
					readers.push_back(make_pair(item->first, terminal->getName()));
				}
			}
		}
	}

	int ServerResource::getReadersInformation(ByteArray &info) const
	{
		int result = 0;
		unsigned char *buffer = NULL;
		unsigned int length = 0;
		unsigned int offset = 0;
		unsigned int nameLen = 0;

		if (mapReaders.size() > 0)
		{
			const Terminal *terminal = NULL;
			map<unsigned int, unsigned int>::const_iterator item;

			for (item = mapReaders.begin(); item != mapReaders.end(); item++)
			{
				if (item->second != IntegerHandle::INVALID_HANDLE)
				{
					terminal = getTerminal(item->second);
					if (terminal != NULL)
					{
						if (terminal->isSecureElementPresence())
						{
							length += sizeof(nameLen) + strlen(terminal->getName()) + sizeof(unsigned int);
							result++;
						}
					}
				}
			}

			buffer = new unsigned char[length];
			if (buffer != NULL)
			{
				memset(buffer, 0, length);

				for (item = mapReaders.begin(); item != mapReaders.end(); item++)
				{
					if (item->second != IntegerHandle::INVALID_HANDLE)
					{
						terminal = getTerminal(item->second);
						if (terminal != NULL)
						{
							if (terminal->isSecureElementPresence())
							{
								nameLen = strlen(terminal->getName());

								memcpy(buffer + offset, &nameLen, sizeof(nameLen));
								offset += sizeof(nameLen);

								memcpy(buffer + offset, terminal->getName(), nameLen);
								offset += nameLen;

								memcpy(buffer + offset, &item->first, sizeof(unsigned int));
								offset += sizeof(unsigned int);
							}
						}
					}
				}

				info.assign(buffer, length);
				delete []buffer;
			}
			else
			{
				_ERR("alloc failed");
				result = -1;
			}
		}
		else
		{
			_INFO("no secure element");
		}

		return result;
	}

#ifndef USE_GDBUS
	bool ServerResource::sendMessageToAllClients(const Message &msg)
	{
		bool result = true;

		map<int, ClientInstance *>::const_iterator item;

		for (item = mapClients.begin();
			item != mapClients.end(); item++)
		{
			if (item->second->sendMessageToAllServices(
				item->second->getSocket(), msg) == false)
				result = false;
		}

		return result;
	}
#endif

	void ServerResource::terminalCallback(const void *terminal, int event,
		int error, void *user_param)
	{
		_DBG("terminal [%s], event [%d], error [%d], user_param [%p]", (char *)terminal, event, error, user_param);

		switch (event)
		{
		case Terminal::NOTIFY_SE_AVAILABLE :
			{
				ServerResource &instance = ServerResource::getInstance();
				unsigned int terminalID = IntegerHandle::INVALID_HANDLE;

				_INFO("[NOTIFY_SE_AVAILABLE]");

				terminalID = instance.getTerminalID((char *)terminal);
				if (terminalID != IntegerHandle::INVALID_HANDLE)
				{
					unsigned int readerID = instance.createReader(terminalID);
#ifdef USE_GDBUS
					ServerGDBus::getInstance().emitReaderInserted(readerID, (const char *)terminal);
#else
					Message msg;

					/* send all client to refresh reader */
					msg.message = msg.MSG_NOTIFY_SE_INSERTED;
					msg.param1 = readerID;
					msg.data.assign((uint8_t *)terminal,
						strlen((char *)terminal) + 1);

					instance.sendMessageToAllClients(msg);
#endif
				}
			}
			break;

		case Terminal::NOTIFY_SE_NOT_AVAILABLE :
			{
				ServerResource &instance = ServerResource::getInstance();
				unsigned int readerID = IntegerHandle::INVALID_HANDLE;

				_INFO("[NOTIFY_SE_NOT_AVAILABLE]");

				readerID = instance.getReaderID((char *)terminal);
#ifdef USE_GDBUS
				ServerGDBus::getInstance().emitReaderRemoved(
					readerID, (const char *)terminal);
#else
				Message msg;

				/* send all client to refresh reader */
				msg.message = msg.MSG_NOTIFY_SE_REMOVED;
				msg.param1 = readerID;
				msg.data.assign((uint8_t *)terminal,
					strlen((char *)terminal) + 1);

				instance.sendMessageToAllClients(msg);
#endif
				instance.removeReader(readerID);
			}
			break;

		default :
			_DBG("terminal [%s], event [%d], error [%d], user_param [%p]", (char *)terminal, event, error, user_param);
			break;
		}
	}

	unsigned int ServerResource::createReader(unsigned int terminalID)
	{
		unsigned int result = -1;

		result = IntegerHandle::assignHandle();

		mapReaders.insert(make_pair(result, terminalID));

		return result;
	}

	unsigned int ServerResource::getReaderID(const char *name) const
	{
		unsigned int result = IntegerHandle::INVALID_HANDLE,
			terminalID = IntegerHandle::INVALID_HANDLE;

		terminalID = getTerminalID(name);
		if (terminalID != IntegerHandle::INVALID_HANDLE)
		{
			map<unsigned int, unsigned int>::const_iterator item;

			for (item = mapReaders.begin();
				item != mapReaders.end(); item++)
			{
				if (item->second == terminalID)
				{
					result = item->first;
					break;
				}
			}
		}

		return result;
	}

	void ServerResource::removeReader(unsigned int readerID)
	{
		map<unsigned int, unsigned int>::iterator item;

		if ((item = mapReaders.find(readerID)) != mapReaders.end())
		{
			item->second = IntegerHandle::INVALID_HANDLE;
		}
	}

	bool ServerResource::isAuthorizedNFCAccess(Terminal *terminal,
		const ByteArray &aid, const vector<ByteArray> &hashes)
	{
		bool result = false;

		if (terminal == NULL) {
			return result;
		}

		int num = _openLogicalChannel(terminal);
		if (num > 0) {
			/* create channel instance */
			ServerChannel *channel = new ServerChannel(NULL, NULL, num, terminal);
			if (channel != NULL) {
				AccessControlList *acl = getAccessControlList(channel);
				if (acl == NULL) {

					/* load access control defined by Global Platform */
					acl = new GPACE();
					if (acl != NULL) {
						int ret;

						ret = acl->loadACL(channel);
						if (ret >= SCARD_ERROR_OK) {
							addAccessControlList(channel, acl);
						} else {
							_ERR("unknown error, 0x%x", -ret);
							delete acl;
						}
					} else {
						_ERR("alloc failed");
					}
				} else {
					acl->updateACL(channel);
				}

				if (acl != NULL) {
					result = acl->isAuthorizedNFCAccess(aid, hashes);
				} else {
					_ERR("acl is null");
				}

				delete channel;
			} else {
				_ERR("alloc failed");
			}
		} else {
			_ERR("_openLogicalChannel failed");
		}

		return result;
	}
} /* namespace smartcard_service_api */

using namespace smartcard_service_api;

EXTERN_API void server_resource_set_main_loop_instance(void *instance)
{
	ServerResource::getInstance().setMainLoopInstance(instance);
}
