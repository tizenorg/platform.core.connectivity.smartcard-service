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

#ifndef GPACE_H_
#define GPACE_H_

#include "smartcard-types.h"
#ifdef __cplusplus
#include "AccessControlList.h"
#endif /* __cplusplus */

#ifdef __cplusplus
namespace smartcard_service_api
{
	class EXPORT GPACE : public AccessControlList
	{
	private :
		AccessControlList *acl;

	public :
		GPACE();
		~GPACE();

		int loadACL(Channel *channel);

		bool isAuthorizedAccess(const ByteArray &aid,
			const ByteArray &certHash) const;
		bool isAuthorizedAccess(const unsigned char *aidBuffer,
			unsigned int aidLength,
			const unsigned char *certHashBuffer,
			unsigned int certHashLength) const;
		bool isAuthorizedAccess(const ByteArray &aid,
			const vector<ByteArray> &certHashes) const;
		bool isAuthorizedAccess(const ByteArray &aid,
			const vector<ByteArray> &certHashes,
			const ByteArray &command) const;
		bool isAuthorizedNFCAccess(const ByteArray &aid,
			const vector<ByteArray> &certHashes) const;
	};

} /* namespace smartcard_service_api */
#endif /* __cplusplus */
#endif /* GPACE_H_ */
