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

#ifndef GPARAACL_H_
#define GPARAACL_H_

#include "smartcard-types.h"
#ifdef __cplusplus
#include "AccessControlList.h"
#include "GPARAM.h"
#endif /* __cplusplus */

#ifdef __cplusplus
namespace smartcard_service_api
{
	class GPARAACL : public AccessControlList
	{
	private:
		ByteArray refreshTag;

		void addCondition(const ByteArray &aid, const ByteArray &hash,
			const vector<ByteArray> &apduRule, const ByteArray &nfcRule);

		int updateRule(const ByteArray &data);

	public:
		GPARAACL();
		~GPARAACL();

		int loadACL(Channel *channel);
		int loadACL(GPARAM &aram);

		bool isAuthorizedAccess(GPARAM &aram, const ByteArray &aid,
			const ByteArray &certHash) const;
		bool isAuthorizedAccess(GPARAM &aram, const ByteArray &aid,
			const ByteArray &certHash,
			const ByteArray &command) const;
		bool isAuthorizedAccess(GPARAM &aram,
			const unsigned char *aidBuffer,
			unsigned int aidLength,
			const unsigned char *certHashBuffer,
			unsigned int certHashLength) const;
		bool isAuthorizedAccess(GPARAM &aram, const ByteArray &aid,
			const vector<ByteArray> &certHashes) const;
		bool isAuthorizedAccess(GPARAM &aram, const ByteArray &aid,
			const vector<ByteArray> &certHashes,
			const ByteArray &command) const;
		bool isAuthorizedNFCAccess(GPARAM &aram, const ByteArray &aid,
			const vector<ByteArray> &certHashes) const;
	};

} /* namespace smartcard_service_api */
#endif /* __cplusplus */

#endif /* GPARAACL_H_ */