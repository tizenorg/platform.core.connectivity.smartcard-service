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

#ifndef DISPATCHERHELPER_H_
#define DISPATCHERHELPER_H_

/* standard library header */
#include <queue>
#include <pthread.h>

/* SLP library header */

/* local header */
#include "Synchronous.h"
#include "DispatcherMsg.h"

using namespace std;

namespace smartcard_service_api
{
	class DispatcherHelper : public Synchronous
	{
	private:
		pthread_t dispatcherThread;

		queue<DispatcherMsg *> messageQ;

		static void *_dispatcherThreadFunc(void *data);

		DispatcherMsg *fetchMessage();

	protected:
		virtual void *dispatcherThreadFunc(DispatcherMsg *msg, void *data) = 0;

	public:
		DispatcherHelper();
		~DispatcherHelper();

		void clearQueue();

		void pushMessage(DispatcherMsg *msg);
		void processMessage(DispatcherMsg *msg);

		bool runDispatcherThread();
		void stopDispatcherThread();

		friend void *_dispatcherThreadFunc(void *data);
	};

} /* namespace smartcard_service_api */
#endif /* DISPATCHERHELPER_H_ */
