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
#include <string.h>

/* SLP library header */

/* local header */
#include "Debug.h"
#include "DispatcherHelper.h"

namespace smartcard_service_api
{
	DispatcherHelper::DispatcherHelper()
	{
		dispatcherThread = 0;
	}

	DispatcherHelper::~DispatcherHelper()
	{
		stopDispatcherThread();
	}

	DispatcherMsg *DispatcherHelper::fetchMessage()
	{
		DispatcherMsg *result = NULL;

		if (messageQ.size() > 0)
		{
			result = messageQ.front();
			messageQ.pop();
		}

		return result;
	}

	void DispatcherHelper::clearQueue()
	{
		DispatcherMsg *temp = NULL;

		while (messageQ.size() > 0)
		{
			temp = fetchMessage();
			delete temp;
		}
	}

	void DispatcherHelper::pushMessage(DispatcherMsg *msg)
	{
		syncLock();

		messageQ.push(msg);

		signalCondition();
		syncUnlock();
	}

	void *DispatcherHelper::_dispatcherThreadFunc(void *data)
	{
		DispatcherMsg *msg = NULL;
		DispatcherHelper *helper = (DispatcherHelper *)data;

		while (1)
		{
			helper->syncLock();
			if ((msg = helper->fetchMessage()) == NULL)
			{
				helper->waitTimedCondition(0);
				helper->syncUnlock();
				continue;
			}
			helper->syncUnlock();

			/* process message */
			helper->dispatcherThreadFunc(msg, data);

			delete msg;
		}

		return (void *)NULL;
	}

	bool DispatcherHelper::runDispatcherThread()
	{
		bool result = false;
		pthread_attr_t attr;

		if (dispatcherThread == 0)
		{
			int ret;

			pthread_attr_init(&attr);
			pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

			if ((ret = pthread_create(&dispatcherThread, &attr, &DispatcherHelper::_dispatcherThreadFunc, this)) != 0)
			{
				SCARD_DEBUG_ERR("pthread_create failed [%d]", ret);
			}
			else
			{
				SCARD_DEBUG("pthread_create success");
				result = true;
			}
		}
		else
		{
			SCARD_DEBUG("thread already start");
			result = true;
		}

		return result;
	}

	void DispatcherHelper::stopDispatcherThread()
	{
		if (dispatcherThread != 0)
		{
			pthread_cancel(dispatcherThread);
			dispatcherThread = 0;
		}
	}

} /* namespace smartcard_service_api */
