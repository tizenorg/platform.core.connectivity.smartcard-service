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

#ifndef CLIENTIPC_H_
#define CLIENTIPC_H_

/* standard library header */
#ifdef USE_AUTOSTART
#include <dbus/dbus-glib.h>
#endif

/* SLP library header */

/* local header */
#include "IPCHelper.h"
#include "SEServiceListener.h"
#ifdef USE_AUTOSTART
#include "smartcard-service-glue.h"
#endif

namespace smartcard_service_api
{
	class ClientIPC: public IPCHelper
	{
	private:
#ifdef SECURITY_SERVER
		ByteArray cookie;
#endif
		ClientIPC();
		~ClientIPC();

#ifdef USE_AUTOSTART
		void _launch_daemon();
#endif
		int handleIOErrorCondition(void *channel, GIOCondition condition);
		int handleInvalidSocketCondition(void *channel, GIOCondition condition);
		int handleIncomingCondition(void *channel, GIOCondition condition);

	public:
		static ClientIPC &getInstance();
		bool sendMessage(Message *msg);
	};

} /* namespace open_mobile_api */
#endif /* CLIENTIPC_H_ */
