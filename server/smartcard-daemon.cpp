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
#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <signal.h>
#ifdef __cplusplus
#include <vector>
#endif /* __cplusplus */
#ifdef USE_AUTOSTART
#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-bindings.h>
#endif

/* SLP library header */

/* local header */
#include "Debug.h"
#include "ServerIPC.h"
#include "ServerResource.h"
#ifdef USE_AUTOSTART
#include "SmartcardDbus.h"
#include "smartcard-service-binding.h"
#endif

/* definition */
#ifdef __cplusplus
using namespace std;
using namespace smartcard_service_api;
#endif /* __cplusplus */

/* global variable */
#ifdef USE_AUTOSTART
GObject *object = NULL;
DBusGConnection *connection = NULL;
#endif

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

#ifdef USE_AUTOSTART
G_DEFINE_TYPE(Smartcard_Service, smartcard_service, G_TYPE_OBJECT)

/* Just Check the assert  and set the error message */
#define __G_ASSERT(test, return_val, error, domain, error_code)\
G_STMT_START\
{\
	if G_LIKELY (!(test)) { \
		g_set_error (error, domain, error_code, #test); \
		return (return_val); \
	}\
}\
G_STMT_END

GQuark smartcard_service_error_quark(void)
{
	SCARD_DEBUG("smartcard_service_error_quark entered");

	return g_quark_from_static_string("smartcard_service_error");
}

static void smartcard_service_init(Smartcard_Service *smartcard_service)
{
	SCARD_DEBUG("smartcard_service_init entered");
}

static void smartcard_service_class_init(Smartcard_ServiceClass *smartcard_service_class)
{
	SCARD_DEBUG("smartcard_service_class_init entered");

	dbus_g_object_type_install_info(SMARTCARD_SERVICE_TYPE, &dbus_glib_smartcard_service_object_info);
}

gboolean smartcard_service_launch(Smartcard_Service *smartcard_service, guint *result_val, GError **error)
{
	SCARD_DEBUG("smartcard_service_launch entered");

	return TRUE;
}

static void _initialize_dbus()
{
	GError *error = NULL;
	DBusGProxy *proxy = NULL;
	guint ret = 0;

	SCARD_BEGIN();

	g_type_init();

	connection = dbus_g_bus_get(DBUS_BUS_SYSTEM, &error);
	if (error == NULL)
	{
		object = (GObject *)g_object_new(SMARTCARD_SERVICE_TYPE, NULL);
		dbus_g_connection_register_g_object(connection, SMARTCARD_SERVICE_PATH, object);

		/* register service */
		proxy = dbus_g_proxy_new_for_name(connection, DBUS_SERVICE_DBUS, DBUS_PATH_DBUS, DBUS_INTERFACE_DBUS);
		if (proxy != NULL)
		{
			if (!org_freedesktop_DBus_request_name(proxy, SMARTCARD_SERVICE_NAME, 0, &ret, &error))
			{
				SCARD_DEBUG_ERR("Unable to register service: %s", error->message);
				g_error_free(error);
			}

			g_object_unref (proxy);
		}
		else
		{
			SCARD_DEBUG_ERR("dbus_g_proxy_new_for_name failed");
		}
	}
	else
	{
		SCARD_DEBUG_ERR("ERROR: Can't get on system bus [%s]", error->message);
		g_error_free(error);
	}

	SCARD_END();
}

static void _finalize_dbus()
{
	SCARD_BEGIN();

	dbus_g_connection_unregister_g_object(connection, object);
	g_object_unref(object);

	SCARD_END();
}
#endif

static void __sighandler(int sig)
{
	SCARD_DEBUG("signal!! [%d]", sig);

#ifdef USE_AUTOSTART
	_finalize_dbus();
#endif
}

int main()
{
	GMainLoop *loop = NULL;

	signal(SIGTERM, &__sighandler);

#ifndef USE_AUTOSTART
	daemonize();
#endif

	if (!g_thread_supported())
	{
		g_thread_init(NULL);
	}

	loop = g_main_new(TRUE);

#ifdef __cplusplus
	ServerResource::getInstance().setMainLoopInstance(loop);
	ServerIPC::getInstance()->createListenSocket();
#else /* __cplusplus */
	server_resource_set_main_loop_instance(loop);
	server_ipc_create_listen_socket();
#endif /* __cplusplus */

#ifdef USE_AUTOSTART
	_initialize_dbus();
#endif
	g_main_loop_run(loop);

#ifdef USE_AUTOSTART
	_finalize_dbus();
#endif
	/* release secure element.. (pure virtual function problem..) */
	ServerResource::getInstance().unloadSecureElements();

	return 0;
}
