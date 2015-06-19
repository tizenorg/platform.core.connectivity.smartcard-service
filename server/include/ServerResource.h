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
#include <map>
#include <vector>
#include <set>
#include <string>

/* SLP library header */

/* local header */
#include "Exception.h"
#include "Terminal.h"
#include "Lock.h"
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
		map<unsigned int, Terminal *> mapTerminals; /* terminal unique id <-> terminal instance map */
		map<unsigned int, unsigned int> mapReaders; /* reader unique id <-> terminal unique id map */
		map<string, ClientInstance *> mapClients; /* client pid <-> client instance map */
		map<Terminal *, AccessControlList *> mapACL; /* terminal instance <-> access control instance map */
		bool seLoaded;

		ServerResource();
		~ServerResource();

		Terminal *createInstance(void *library);
		bool appendSELibrary(char *library);
		void clearSELibraries();

		static void terminalCallback(const void *terminal, int event, int error, void *user_param);

		int _openLogicalChannel(Terminal *terminal);
		int _closeLogicalChannel(Terminal *terminal, int channelNum);
		bool _isAuthorizedAccess(ServerChannel *channel, const ByteArray &aid, const vector<ByteArray> &hashes);
		unsigned int _createChannel(Terminal *terminal, ServiceInstance *service, int channelType, unsigned int sessionID, const ByteArray &aid)
			throw(ExceptionBase &);

	public:
		/* static member */
		static ServerResource &getInstance();

		int loadSecureElements();
		void unloadSecureElements();

		Terminal *getTerminal(unsigned int terminalID);
		const Terminal *getTerminal(unsigned int terminalID) const;
		Terminal *getTerminal(const char *name);
		Terminal *getTerminalByReaderID(unsigned int readerID);
		const Terminal *getTerminalByReaderID(unsigned int readerID) const;
		unsigned int getTerminalID(const char *name) const;
		int getReadersInformation(ByteArray &info) const;
		void getReaders(vector<pair<unsigned int, string> > &readers) const;
		bool isValidReaderHandle(unsigned int reader) const;

		unsigned int createReader(unsigned int terminalID);
		unsigned int getReaderID(const char *name) const;
		void removeReader(unsigned int readerID);

		bool createClient(const char *name, pid_t pid);
		ClientInstance *getClient(const char *name);
		void removeClient(const char *name);
		void removeClients();
		int getClientCount() const;

		ServiceInstance *createService(const char *name);
		ServiceInstance *getService(const char *name, unsigned int handle);
		void removeService(const char *name, unsigned int handle);
		void removeServices(const char *name);

		unsigned int createSession(const char *name, unsigned int handle, unsigned int readerID, vector<ByteArray> &certHashes, void *caller);
		ServerSession *getSession(const char *name, unsigned int handle, unsigned int sessionID);
		unsigned int getChannelCount(const char *name, unsigned int handle, unsigned int sessionID);
		void removeSession(const char *name, unsigned int handle, unsigned int session);
		bool isValidSessionHandle(const char *name, unsigned int handle, unsigned int sessionID);
		bool isValidChannelHandle(const char *name, unsigned int handle, unsigned int channelID);

		unsigned int createChannel(const char *name, unsigned int handle,
			unsigned int sessionID, int channelType, ByteArray aid)
			throw(ExceptionBase &);
		Channel *getChannel(const char *name, unsigned int handle, unsigned int channelID);
		void removeChannel(const char *name, unsigned int handle, unsigned int channelID);
		void addAccessControlList(Terminal *terminal, AccessControlList *acl);
		void addAccessControlList(ServerChannel *channel, AccessControlList *acl);
		AccessControlList *getAccessControlList(Terminal *terminal);
		AccessControlList *getAccessControlList(ServerChannel *channel);
		bool isAuthorizedNFCAccess(Terminal *terminal, const ByteArray &aid,
			const vector<ByteArray> &hashes);
		bool isAuthorizedAccess(Terminal *terminal,
			const ByteArray &aid, const vector<ByteArray> &hashes);

		void finish();

		/* FIXME ??? */
		ServerChannel *createInternalChannel(Terminal *terminal,
			int channelType);

		friend void terminalCallback(void *terminal, int event, int error, void *user_param);
	};

} /* namespace smartcard_service_api */
#endif /* SERVERRESOURCE_H_ */
