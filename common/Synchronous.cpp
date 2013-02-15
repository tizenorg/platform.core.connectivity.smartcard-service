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
#include <sys/time.h>

/* SLP library header */

/* local header */
#include "Synchronous.h"

namespace smartcard_service_api
{
	Synchronous::Synchronous()
	{
		pthread_mutex_init(&syncMutex, NULL);
		pthread_cond_init(&syncCondition, NULL);
	}

	void Synchronous::syncLock()
	{
		pthread_mutex_lock(&syncMutex);
	}

	void Synchronous::syncUnlock()
	{
		pthread_mutex_unlock(&syncMutex);
	}

	int Synchronous::waitTimedCondition(int second)
	{
		struct timeval now;
		struct timespec ts;
		int result;

		if (second > 0)
		{
			gettimeofday(&now, NULL);
			ts.tv_sec = now.tv_sec + second;
			ts.tv_nsec = now.tv_usec * 1000;

			result = pthread_cond_timedwait(&syncCondition, &syncMutex, &ts);
		}
		else
		{
			result = pthread_cond_wait(&syncCondition, &syncMutex);
		}

		return result;
	}

	void Synchronous::signalCondition()
	{
		pthread_cond_signal(&syncCondition);
	}

} /* namespace smartcard_service_api */
