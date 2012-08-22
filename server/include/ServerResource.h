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


#ifndef SERVERRESOURCE_H_
#define SERVERRESOURCE_H_

/* standard library header */
#include <map>
#include <vector>
#include <set>

/* SLP library header */

/* local header */
#include "Terminal.h"
#include "Lock.h"
#include "ServerIPC.h"
#include "ServerDispatcher.h"
#include "ServerReader.h"
#include "ServerSession.h"
#include "ClientInstance.h"
#include "ServiceInstance.h"

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
		map<unsigned int, Terminal *> mapTerminals; /* unique id <-> terminal instance map */
		map<int, ClientInstance *> mapClients; /* client pid <-> client instance map */
		map<Terminal *, AccessControlList *> mapACL; /* terminal instance <-> access control instance map */

		ServerIPC *serverIPC;
		ServerDispatcher *serverDispatcher;

		ServerResource();
		~ServerResource();

		Terminal *createInstance(void *library);
		bool appendSELibrary(char *library);
		void clearSELibraries();

		static void terminalCallback(void *terminal, int event, int error, void *user_param);

	public:
		/* static member */
		static ServerResource &getInstance();

		/* non-static member */
		int loadSecureElements();
		void unloadSecureElements();

		Terminal *getTerminal(unsigned int terminalID);
		Terminal *getTerminal(const char *name);
		int getReadersInformation(ByteArray &info);
		bool isValidReaderHandle(unsigned int reader);

		bool createClient(void *ioChannel, int socket, int watchID, int state, int pid);
		ClientInstance *getClient(int socket);
		void setPID(int socket, int pid);
		void removeClient(int socket);
		void removeClients();

		bool createService(int socket, unsigned int context);
		ServiceInstance *getService(int socket, unsigned int context);
		void removeService(int socket, unsigned int context);
		void removeServices(int socket);

		unsigned int createSession(int socket, unsigned int context, unsigned int terminalID, ByteArray packageCert, void *caller);
		ServerSession *getSession(int socket, unsigned int context, unsigned int sessionID);
		unsigned int getChannelCount(int socket, unsigned int context, unsigned int sessionID);
		void removeSession(int socket, unsigned int context, unsigned int session);
		bool isValidSessionHandle(int socket, unsigned int context, unsigned int sessionID);

		unsigned int createChannel(int socket, unsigned int context, unsigned int sessionID, int channelType, ByteArray aid);
		Channel *getChannel(int socket, unsigned int context, unsigned int channelID);
		void removeChannel(int socket, unsigned int context, unsigned int channelID);

		AccessControlList *getAccessControlList(Terminal *terminal);

		bool sendMessageToAllClients(Message &msg);

		friend void terminalCallback(void *terminal, int event, int error, void *user_param);
	};

} /* namespace smartcard_service_api */
#endif /* SERVERRESOURCE_H_ */
