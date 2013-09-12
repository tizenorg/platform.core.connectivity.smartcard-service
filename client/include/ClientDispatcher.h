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

#ifndef CLIENTDISPATCHER_H_
#define CLIENTDISPATCHER_H_
#ifndef USE_GDBUS
/* standard library header */
#include <map>

/* SLP library header */

/* local header */
#include "DispatcherHelper.h"

using namespace std;

namespace smartcard_service_api
{
	class SEService;

	class ClientDispatcher: public DispatcherHelper
	{
	private:
		map<unsigned int, SEService *> mapSESerivces;

		ClientDispatcher();
		~ClientDispatcher();

		void *dispatcherThreadFunc(DispatcherMsg *msg, void *data);

	public:
		static ClientDispatcher &getInstance();

		bool addSEService(unsigned int handle, SEService *service);
		void removeSEService(unsigned int handle);
	};

} /* namespace open_mobile_api */
#endif /* USE_GDBUS */
#endif /* CLIENTDISPATCHER_H_ */
