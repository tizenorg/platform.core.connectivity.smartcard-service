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

#ifndef SERVERRESOURCE_H_
#define SERVERRESOURCE_H_

/* standard library header */
#ifdef __cplusplus
#include <map>
#include <vector>
#include <set>
#endif /* __cplusplus */

/* SLP library header */

/* local header */
#ifdef __cplusplus
#include "Exception.h"
#include "Terminal.h"
#include "Lock.h"
#include "ServerIPC.h"
#include "ServerDispatcher.h"
#include "ServerReader.h"
#include "ServerSession.h"
#include "ClientInstance.h"
#include "ServiceInstance.h"
#endif /* __cplusplus */

#ifdef __cplusplus
using namespace std;

namespace smartcard_service_api
{
	class IntegerHandle
	{
	private:
		static unsigned int newHandle;
		static set<unsigned int> setHandles;
		static PMutex mutexLock;

	public:
		static const unsigned int INVALID_HANDLE = (unsigned int)-1;

		static unsigned int assignHandle();
		static void releaseHandle(unsigned int);
	};

	class ServerResource
	{
	private:
		/* non-static member */
		vector<void *> libraries;
		map<unsigned int, Terminal *> mapTerminals; /* terminal unique id <-> terminal instance map */
		map<unsigned int, unsigned int> mapReaders; /* reader unique id <-> terminal unique id map */
		map<int, ClientInstance *> mapClients; /* client pid <-> client instance map */
		map<Terminal *, AccessControlList *> mapACL; /* terminal instance <-> access control instance map */
		void *mainLoop;
		ServerIPC *serverIPC;
		ServerDispatcher *serverDispatcher;
		bool seLoaded;

		ServerResource();
		~ServerResource();

		Terminal *createInstance(void *library);
		bool appendSELibrary(char *library);
		void clearSELibraries();

		static void terminalCallback(void *terminal, int event, int error, void *user_param);

		int _openLogicalChannel(Terminal *terminal);
		int _closeLogicalChannel(Terminal *terminal, int channelNum);
		bool _isAuthorizedAccess(ServerChannel *channel, int pid, ByteArray aid, vector<ByteArray> &hashes);
		unsigned int _createChannel(Terminal *terminal, ServiceInstance *service, int channelType, unsigned int sessionID, ByteArray aid)
			throw(ExceptionBase &);
	public:
		/* static member */
		static ServerResource &getInstance();

		/* non-static member */
		inline void setMainLoopInstance(void *mainLoop)
		{
			this->mainLoop = mainLoop;
		}
		inline void *getMainLoopInstance()
		{
			return this->mainLoop;
		}

		int loadSecureElements();
		void unloadSecureElements();

		Terminal *getTerminal(unsigned int terminalID);
		Terminal *getTerminal(const char *name);
		Terminal *getTerminalByReaderID(unsigned int readerID);
		unsigned int getTerminalID(const char *name);
		int getReadersInformation(ByteArray &info);
		bool isValidReaderHandle(unsigned int reader);

		unsigned int createReader(unsigned int terminalID);
		unsigned int getReaderID(const char *name);
		void removeReader(unsigned int readerID);

		bool createClient(void *ioChannel, int socket, int watchID, int state, int pid);
		ClientInstance *getClient(int socket);
		void setPID(int socket, int pid);
		int getClientCount();
		void removeClient(int socket);
		void removeClients();

		ServiceInstance *createService(int socket);
		ServiceInstance *getService(int socket, unsigned int handle);
		void removeService(int socket, unsigned int handle);
		void removeServices(int socket);

		unsigned int createSession(int socket, unsigned int handle, unsigned int readerID, vector<ByteArray> &certHashes, void *caller);
		ServerSession *getSession(int socket, unsigned int handle, unsigned int sessionID);
		unsigned int getChannelCount(int socket, unsigned int handle, unsigned int sessionID);
		void removeSession(int socket, unsigned int handle, unsigned int session);
		bool isValidSessionHandle(int socket, unsigned int handle, unsigned int sessionID);

		unsigned int createChannel(int socket, unsigned int handle,
			unsigned int sessionID, int channelType, ByteArray aid)
			throw(ExceptionBase &);
		Channel *getChannel(int socket, unsigned int handle, unsigned int channelID);
		void removeChannel(int socket, unsigned int handle, unsigned int channelID);

		void addAccessControlList(Terminal *terminal, AccessControlList *acl);
		void addAccessControlList(ServerChannel *channel, AccessControlList *acl);
		AccessControlList *getAccessControlList(Terminal *terminal);
		AccessControlList *getAccessControlList(ServerChannel *channel);

		bool sendMessageToAllClients(Message &msg);

		bool isAuthorizedNFCAccess(Terminal *terminal, ByteArray &aid,
			vector<ByteArray> &hashes);

		friend void terminalCallback(void *terminal, int event, int error, void *user_param);
	};

} /* namespace smartcard_service_api */
#endif /* __cplusplus */

/* export C API */
#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

void server_resource_set_main_loop_instance(void *instance);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* SERVERRESOURCE_H_ */
