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

#ifndef LIBSCL_EXPORT_API
#define LIBSCL_EXPORT_API
#endif // LIBSCL_EXPORT_API


using namespace std;

namespace smartcard_service_api
{
	class LIBSCL_EXPORT_API AccessControlList
	{
	protected:
		map<ByteArray, AccessCondition> mapConditions;
		bool allGranted;

		const AccessRule *findAccessRule(const ByteArray &aid,
			const ByteArray &hash) const;
		AccessCondition &getAccessCondition(const ByteArray &aid);

		void printAccessControlList() const;

	public:
		static ByteArray ALL_SE_APPS;
		static ByteArray DEFAULT_SE_APP;
		static ByteArray ALL_DEVICE_APPS;

		AccessControlList();
		virtual ~AccessControlList();

		virtual int loadACL(Channel *channel) = 0;

		int updateACL(Channel *channel) { return loadACL(channel); }
		void releaseACL();

		/* FIXME ??? */
		inline bool hasConditions() const { return mapConditions.size() > 0; }

		virtual bool isAuthorizedAccess(const ByteArray &aid,
			const ByteArray &certHash) const;
		virtual bool isAuthorizedAccess(const unsigned char *aidBuffer,
			unsigned int aidLength, const unsigned char *certHashBuffer,
			unsigned int certHashLength) const;
		virtual bool isAuthorizedAccess(const ByteArray &aid,
			const vector<ByteArray> &certHashes) const;
		virtual bool isAuthorizedAccess(const ByteArray &aid,
			const vector<ByteArray> &certHashes, const ByteArray &command) const;
		virtual bool isAuthorizedNFCAccess(const ByteArray &aid,
			const vector<ByteArray> &certHashes) const;
	};

} /* namespace smartcard_service_api */
#endif /* ACCESSCONTROLLIST_H_ */
