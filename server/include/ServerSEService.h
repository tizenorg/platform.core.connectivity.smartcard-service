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

#ifndef SERVERSESERVICE_H_
#define SERVERSESERVICE_H_

/* standard library header */
#include <vector>

/* SLP library header */

/* local header */
#include "SEServiceHelper.h"
#include "Terminal.h"
#include "ServerReader.h"

using namespace std;

namespace smartcard_service_api
{
	class ServerSEService : public SEServiceHelper
	{
	private:
		vector<void *> libraries;
		map<char *, Terminal *> mapTerminals;

		ServerSEService();
		~ServerSEService();

		Terminal *createInstance(void *library);
		bool appendSELibrary(char *library);

		int openSELibraries();
		void closeSELibraries();

		static void terminalCallback(void *terminal, int event, int error, void *user_param);
		static bool dispatcherCallback(void *message, int socket);

	public:
		static ServerSEService &getInstance();

#if 0
		bool isValidReaderHandle(void *handle);
#endif

		void shutdown() {}
		void shutdownSync() {}
		friend void terminalCallback(char *name, int event, int error, void *user_param);
		friend bool dispatcherCallback(void *message, int socket);
		friend class ServerDispatcher;
	};

} /* namespace smartcard_service_api */
#endif /* SERVERSESERVICE_H_ */
