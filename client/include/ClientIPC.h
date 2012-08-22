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

#ifndef CLIENTIPC_H_
#define CLIENTIPC_H_

/* standard library header */

/* SLP library header */

/* local header */
#include "IPCHelper.h"
#include "SEServiceListener.h"

namespace smartcard_service_api
{
	class ClientIPC: public IPCHelper
	{
	private:
		ClientIPC();
		~ClientIPC();

		int handleIOErrorCondition(void *channel, GIOCondition condition);
		int handleInvalidSocketCondition(void *channel, GIOCondition condition);
		int handleIncomingCondition(void *channel, GIOCondition condition);

	public:
		static ClientIPC &getInstance();
	};

} /* namespace open_mobile_api */
#endif /* CLIENTIPC_H_ */
