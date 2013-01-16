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


#ifndef READERHELPER_H_
#define READERHELPER_H_

/* standard library header */
#include <vector>

/* SLP library header */

/* local header */
#include "Synchronous.h"
#include "SessionHelper.h"

using namespace std;

namespace smartcard_service_api
{
	class SEServiceHelper;

	typedef void (*openSessionCallback)(SessionHelper *session, int error, void *userData);

	class ReaderHelper : public Synchronous
	{
	protected:
		char name[30];
		vector<SessionHelper *> sessions;
		SEServiceHelper *seService;
		bool present;

		ReaderHelper();

	public:
		virtual ~ReaderHelper() {}

		inline const char *getName() { return name; }
		inline SEServiceHelper *getSEService() { return seService; }
		inline bool isSecureElementPresent() { return present; }

		virtual void closeSessions() = 0;

		virtual int openSession(openSessionCallback callback, void *userData) = 0;
		virtual SessionHelper *openSessionSync() = 0;
	};

} /* namespace smartcard_service_api */
#endif /* READERHELPER_H_ */
