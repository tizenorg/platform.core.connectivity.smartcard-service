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
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>

/* SLP library header */

/* local header */
#include "Debug.h"
#include "ServerSEService.h"
#include "ServerReader.h"
#include "GPSEACL.h"

namespace smartcard_service_api
{
	ServerReader::ServerReader(ServerSEService *seService, char *name, Terminal *terminal) :
		ReaderHelper()
	{
		unsigned int length = 0;

		acList = NULL;

		if (seService == NULL || name == NULL || strlen(name) == 0 || terminal == NULL)
		{
			_ERR("invalid param");

			return;
		}

		this->terminal = terminal;
		this->seService = seService;

		length = strlen(name);
		length = (length < sizeof(this->name)) ? length : sizeof(this->name);
		memcpy(this->name, name, length);

		/* open admin channel */
		adminChannel = new ServerChannel(NULL, NULL, 0, terminal);
		if (adminChannel == NULL)
		{
			_ERR("alloc failed");
		}
	}

	ServerReader::~ServerReader()
	{
		closeSessions();

		if (acList != NULL)
		{
			delete acList;
			acList = NULL;
		}

		if (adminChannel != NULL)
		{
			delete adminChannel;
			adminChannel = NULL;
		}
	}

	void ServerReader::closeSessions()
		throw(ErrorIO &, ErrorIllegalState &)
	{
		size_t i;

		for (i = 0; i < sessions.size(); i++)
		{
			if (sessions[i] != NULL)
				((ServerSession *)sessions[i])->closeSync();
		}

		sessions.clear();
	}

	AccessControlList *ServerReader::getAccessControlList()
	{
		if (acList == NULL)
		{
			/* load access control */
			acList = new GPSEACL();
			if (acList != NULL)
			{
				acList->loadACL(adminChannel);
			}
			else
			{
				_ERR("alloc failed");
			}
		}

		return acList;
	}

	ServerSession *ServerReader::openSessionSync()
		throw(ErrorIO &, ErrorIllegalState &, ErrorIllegalParameter &, ErrorSecurity &)
	{
		vector<ByteArray> temp;

		return openSessionSync(temp, NULL);
	}

	ServerSession *ServerReader::openSessionSync(vector<ByteArray> &certHashes, void *caller)
		throw(ErrorIO &, ErrorIllegalState &, ErrorIllegalParameter &, ErrorSecurity &)
	{
		ServerSession *session = NULL;

		session = new ServerSession(this, certHashes, caller, terminal);
		if (session == NULL)
			return session;

		sessions.push_back(session);

		return session;
	}

} /* namespace smartcard_service_api */
