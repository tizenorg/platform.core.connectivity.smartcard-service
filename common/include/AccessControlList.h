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


#ifndef ACCESSCONTROLLIST_H_
#define ACCESSCONTROLLIST_H_

/* standard library header */
#include <vector>
#include <map>

/* SLP library header */

/* local header */
#include "ByteArray.h"
#include "Channel.h"

using namespace std;

namespace smartcard_service_api
{
	class AccessCondition;

	class AccessControlList
	{
	protected:
		map<ByteArray, AccessCondition> mapConditions;
		bool allGranted;

		void printAccessControlList();
		bool isAuthorizedAccess(ByteArray aid, ByteArray certHash, bool update);

	public:
		static ByteArray AID_ALL;
		static ByteArray AID_DEFAULT;

		AccessControlList();
		virtual ~AccessControlList();

		virtual int loadACL(Channel *channel) = 0;

		int updateACL(Channel *channel) { return loadACL(channel); }
		void releaseACL();

		bool isAuthorizedAccess(ByteArray aid, ByteArray certHash);
		bool isAuthorizedAccess(unsigned char *aidBuffer, unsigned int aidLength, unsigned char *certHashBuffer, unsigned int certHashLength);
		bool isAuthorizedAccess(ByteArray aid, vector<ByteArray> &certHashes);
	};

} /* namespace smartcard_service_api */
#endif /* ACCESSCONTROLLIST_H_ */
