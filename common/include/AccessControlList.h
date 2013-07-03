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

#ifndef ACCESSCONTROLLIST_H_
#define ACCESSCONTROLLIST_H_

/* standard library header */
#include <vector>
#include <map>

/* SLP library header */

/* local header */
#include "ByteArray.h"
#include "Channel.h"
#include "AccessCondition.h"

using namespace std;

namespace smartcard_service_api
{
	class AccessControlList
	{
	protected:
		map<ByteArray, AccessCondition> mapConditions;
		bool allGranted;

		AccessRule *findAccessRule(const ByteArray &aid,
			const ByteArray &hash);
		AccessCondition &getAccessCondition(const ByteArray &aid);

		void printAccessControlList();

	public:
		static ByteArray ALL_SE_APPS;
		static ByteArray DEFAULT_SE_APP;
		static ByteArray ALL_DEVICE_APPS;

		AccessControlList();
		virtual ~AccessControlList();

		virtual int loadACL(Channel *channel) = 0;

		int updateACL(Channel *channel) { return loadACL(channel); }
		void releaseACL();

		virtual bool isAuthorizedAccess(ByteArray &aid,
			ByteArray &certHash);
		virtual bool isAuthorizedAccess(unsigned char *aidBuffer,
			unsigned int aidLength, unsigned char *certHashBuffer,
			unsigned int certHashLength);
		virtual bool isAuthorizedAccess(ByteArray &aid,
			vector<ByteArray> &certHashes);
		virtual bool isAuthorizedAccess(ByteArray &aid,
			vector<ByteArray> &certHashes, ByteArray &command);
		virtual bool isAuthorizedNFCAccess(ByteArray &aid,
			vector<ByteArray> &certHashes);
	};

} /* namespace smartcard_service_api */
#endif /* ACCESSCONTROLLIST_H_ */
