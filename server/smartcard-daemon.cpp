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
#include <unistd.h>
#include <string.h>
#include <glib.h>
#include <vector>
#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>

/* SLP library header */

/* local header */
#include "Debug.h"
#include "Channel.h"
#include "ServerResource.h"
#include "ServerSEService.h"

/* definition */
using namespace std;
using namespace smartcard_service_api;

/* global variable */

ServerResource *serverResource = NULL;

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

int main()
{
	GMainLoop* loop = NULL;

	daemonize();

	if (!g_thread_supported())
	{
		g_thread_init(NULL);
	}

	serverResource = &ServerResource::getInstance();
	ServerIPC::getInstance()->createListenSocket();

	loop = g_main_new(TRUE);
	g_main_loop_run(loop);

	return 0;
}
