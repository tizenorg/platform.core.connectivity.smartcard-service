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

#ifndef SERVERGDBUS_H_
#define SERVERGDBUS_H_

#ifdef USE_GDBUS
#include <glib.h>

#include "smartcard-service-gdbus.h"

namespace smartcard_service_api
{
	class ServerGDBus
	{
	public :
		GDBusProxy *dbus_proxy;

		static ServerGDBus &getInstance();

		bool init();
		void deinit();

		/* connect to dbus daemon */
		bool _init();
		void _deinit();
		pid_t getPID(const char *name);

		void emitReaderInserted(unsigned int reader_id,
			const char *reader_name);
		void emitReaderRemoved(unsigned int reader_id,
			const char *reader_name);

	private :

		GDBusConnection *connection;

		SmartcardServiceSeService *seService;
		SmartcardServiceReader *reader;
		SmartcardServiceSession *session;
		SmartcardServiceChannel *channel;

		ServerGDBus();
		~ServerGDBus();

		static void name_owner_changed(GDBusProxy *proxy,
			const gchar *name, const gchar *old_owner,
			const gchar *new_owner, void *user_data);

		bool initSEService();
		void deinitSEService();

		bool initReader();
		void deinitReader();

		bool initSession();
		void deinitSession();

		bool initChannel();
		void deinitChannel();
	};
} /* namespace smartcard_service_api */
#endif
#endif /* SERVERGDBUS_H_ */
