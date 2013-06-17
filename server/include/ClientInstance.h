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

#ifndef CLIENTINSTANCE_H_
#define CLIENTINSTANCE_H_

/* standard library header */
#include <map>
#include <vector>
#ifndef USE_GDBUS
#include <glib.h>
#endif

/* SLP library header */

/* local header */
#include "Message.h"
#include "ServiceInstance.h"

namespace smartcard_service_api
{
	class ClientInstance
	{
	private :
#ifdef USE_GDBUS
		string name;
#else
		void *ioChannel;
		int socket;
		int watchID;
		int state;
#endif
		pid_t pid;
		vector<ByteArray> certHashes;
		map<unsigned int, ServiceInstance *> mapServices;

	public :
#ifdef USE_GDBUS
		ClientInstance(const char *name, pid_t pid) : name(name), pid(pid)
		{
		}
#else
		ClientInstance(void *ioChannel, int socket, int watchID,
			int state, int pid) : ioChannel(ioChannel),
			socket(socket), watchID(watchID), state(state), pid(pid)
		{
		}

		ClientInstance(pid_t pid) : ioChannel(NULL),
			socket(pid), watchID(0), state(0), pid(pid)
		{
		}
#endif
		~ClientInstance() { removeServices(); }
#ifdef USE_GDBUS
		inline bool operator ==(const char *name) const { return (this->name.compare(name) == 0); }
#else
		inline bool operator ==(const int &socket) const { return (this->socket == socket); }
#endif

#ifndef USE_GDBUS
		inline void *getIOChannel() { return ioChannel; }
		inline int getSocket() { return socket; }
		inline int getWatchID() { return watchID; }
		inline int getState() { return state; }
#endif
		void setPID(int pid);
		inline int getPID() { return pid; }

		ServiceInstance *createService();
		ServiceInstance *getService(unsigned int handle);
		void removeService(unsigned int handle);
		void removeServices();
#ifndef USE_GDBUS
		bool sendMessageToAllServices(int socket, Message &msg);
#endif
		void generateCertificationHashes();

		inline vector<ByteArray> &getCertificationHashes() { return certHashes; }
	};
} /* namespace smartcard_service_api */
#endif /* CLIENTINSTANCE_H_ */
