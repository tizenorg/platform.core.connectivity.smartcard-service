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
#include <string>

/* SLP library header */

/* local header */
#include "ServiceInstance.h"

namespace smartcard_service_api
{
	class ClientInstance
	{
	private :
		string name;
		pid_t pid;
		vector<ByteArray> certHashes;
		map<unsigned int, ServiceInstance *> mapServices;

	public :
		ClientInstance(const char *name, pid_t pid) :
			name(name), pid(pid)
		{
		}
		inline ~ClientInstance() { removeServices(); }
		inline bool operator ==(const char *name) const { return (this->name.compare(name) == 0); }

		inline void setPID(int pid) { this->pid = pid; }
		inline int getPID() const { return pid; }

		ServiceInstance *createService();
		ServiceInstance *getService(unsigned int handle);
		void removeService(unsigned int handle);
		void removeServices();
		inline size_t getServiceCounts() const { return mapServices.size(); }
		void generateCertificationHashes();

		inline vector<ByteArray> &getCertificationHashes() { return certHashes; }
	};
} /* namespace smartcard_service_api */

#endif /* CLIENTINSTANCE_H_ */
