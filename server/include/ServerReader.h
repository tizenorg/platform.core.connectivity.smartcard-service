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

#ifndef SERVERREADER_H_
#define SERVERREADER_H_

/* standard library header */
#include <vector>

/* SLP library header */

/* local header */
#include "Terminal.h"
#include "ReaderHelper.h"
#include "ServerSession.h"
#include "ServerChannel.h"
#include "AccessControlList.h"

using namespace std;

namespace smartcard_service_api
{
	class ServerSEService;

	class ServerReader : public ReaderHelper
	{
	private:
		Terminal *terminal;

		ServerReader(ServerSEService *seService, const char *name, Terminal *terminal);
		~ServerReader();

		int openSession(openSessionCallback callback, void *userData) { return -1; }
		int openSession(void *caller, openSessionCallback callback, void *userData) { return -1; }

	public:
		void closeSessions()
			throw(ErrorIO &, ErrorIllegalState &);

		ServerSession *openSessionSync()
			throw(ErrorIO &, ErrorIllegalState &, ErrorIllegalParameter &, ErrorSecurity &);
		ServerSession *openSessionSync(const vector<ByteArray> &certHashes, void *caller)
			throw(ErrorIO &, ErrorIllegalState &, ErrorIllegalParameter &, ErrorSecurity &);

		friend class ServerSEService;
	};

} /* namespace smartcard_service_api */
#endif /* SERVERREADER_H_ */
