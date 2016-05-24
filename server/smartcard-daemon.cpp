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
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <glib.h>
#include <glib-object.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <signal.h>
#include <vector>

/* SLP library header */

/* local header */
#include "Debug.h"
#include "ServerResource.h"
#include "smartcard-service-gdbus.h"
#include "ServerGDBus.h"

/* definition */
using namespace std;
using namespace smartcard_service_api;

/* global variable */
GMainLoop *main_loop = NULL;

#ifndef USE_AUTOSTART
static void daemonize(void)
{
	pid_t pid, sid;

	/* already a daemon */
	if (getppid() == 1)
		return;

	/* Fork off the parent process */
	pid = fork();
	if (pid < 0)
	{
		exit(EXIT_FAILURE);
	}

	if (pid > 0)
	{
		exit(EXIT_SUCCESS); /*Killing the Parent Process*/
	}

	/* At this point we are executing as the child process */
	umask(0);

	/* Create a new SID for the child process */
	sid = setsid();
	if (sid < 0)
	{
		exit(EXIT_FAILURE);
	}

	/* Change the current working directory. */
	if ((chdir("/")) < 0)
	{
		exit(EXIT_FAILURE);
	}

	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);
}
#endif

static void _bus_acquired_cb(GDBusConnection *connection,
	const gchar *path, gpointer user_data)
{
	_DBG("bus path : %s", path);

	ServerResource::getInstance();

	ServerGDBus::getInstance().init();
}

static void _name_acquired_cb(GDBusConnection *connection,
	const gchar *name, gpointer user_data)
{
	_DBG("name : %s", name);
}

static void _name_lost_cb(GDBusConnection *connnection,
	const gchar *name, gpointer user_data)
{
	_DBG("name : %s", name);

	ServerGDBus::getInstance().deinit();
}

static void __sighandler(int sig)
{
	_DBG("signal!! [%d]", sig);
}

int main(int argc, char *argv[])
{
	guint id = 0;
	signal(SIGTERM, &__sighandler);

#ifndef USE_AUTOSTART
	daemonize();
#endif

	main_loop = g_main_loop_new(NULL, FALSE);

	id = g_bus_own_name(G_BUS_TYPE_SYSTEM,
			"org.tizen.SmartcardService",
			G_BUS_NAME_OWNER_FLAGS_NONE,
			_bus_acquired_cb,
			_name_acquired_cb,
			_name_lost_cb,
			NULL,
			NULL);

	g_main_loop_run(main_loop);

	if (id)
		g_bus_unown_name(id);

	/* release secure element.. (pure virtual function problem..) */
	ServerResource::getInstance().unloadSecureElements();

	return 0;
}

void smartcard_daemon_exit()
{
	g_main_loop_quit(main_loop);
	g_main_loop_unref(main_loop);
}
