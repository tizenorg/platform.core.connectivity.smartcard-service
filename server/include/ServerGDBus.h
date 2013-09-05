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

/* standard library header */
#include <glib.h>
#include <queue>
#include <vector>

/* SLP library header */

/* local header */
#include "Synchronous.h"
#include "GDBusHelper.h"
#include "smartcard-service-gdbus.h"

using namespace std;

namespace smartcard_service_api
{
	typedef void (*dispatcher_cb_t)(vector<void *> &params);

	class GDBusDispatcher : public Synchronous
	{
	public :
		static GDBusDispatcher &getInstance();

		/* push to queue */
		static void push(dispatcher_cb_t cb, const vector<void *> &params);

	private :
		std::queue<pair<dispatcher_cb_t, vector<void *> > > q;

		GDBusDispatcher();
		~GDBusDispatcher();

		void _push(dispatcher_cb_t cb, const vector<void *> &params);
		static gboolean dispatch(gpointer user_data);
	};

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

#endif /* SERVERGDBUS_H_ */
