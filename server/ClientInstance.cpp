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
#include <pthread.h>

/* SLP library header */

/* local header */
#include "Debug.h"
#include "ClientInstance.h"
#include "ServerResource.h"
#include "SignatureHelper.h"

namespace smartcard_service_api
{
	void ClientInstance::setPID(int pid)
	{
		this->pid = pid;
	}

	ServiceInstance *ClientInstance::createService()
	{
		ServiceInstance *result = NULL;

		result = new ServiceInstance(this);
		if (result != NULL)
		{
			mapServices.insert(make_pair(result->getHandle(), result));
		}
		else
		{
			_ERR("alloc failed");
		}

		return result;
	}

	ServiceInstance *ClientInstance::getService(unsigned int handle)
	{
		ServiceInstance *result = NULL;
		map<unsigned int, ServiceInstance *>::iterator item;

		if ((item = mapServices.find(handle)) != mapServices.end())
		{
			result = item->second;
		}

		return result;
	}

	void ClientInstance::removeService(unsigned int handle)
	{
		map<unsigned int, ServiceInstance *>::iterator item;

		if ((item = mapServices.find(handle)) != mapServices.end())
		{
			delete item->second;
			mapServices.erase(item);
		}
	}

	void ClientInstance::removeServices()
	{
		map<unsigned int, ServiceInstance *>::iterator item;

		for (item = mapServices.begin(); item != mapServices.end(); item++)
		{
			delete item->second;
		}

		mapServices.clear();
	}

	bool ClientInstance::sendMessageToAllServices(int socket, Message &msg)
	{
		bool result = true;
		map<unsigned int, ServiceInstance *>::iterator item;

		for (item = mapServices.begin(); item != mapServices.end(); item++)
		{
			if (ServerIPC::getInstance()->sendMessage(socket, &msg) == false)
				result = false;
		}

		return result;
	}

	void ClientInstance::generateCertificationHashes()
	{
		SignatureHelper::getCertificationHashes(getPID(), certHashes);
	}
} /* namespace smartcard_service_api */
