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

#ifndef ACCESSCONDITION_H_
#define ACCESSCONDITION_H_

#include <map>
#include <vector>

#include "ByteArray.h"

using namespace std;

namespace smartcard_service_api
{
	class AccessRule
	{
	private :
		bool apduRule;
		bool nfcRule;
		vector<pair<ByteArray, ByteArray> > listFilters;

	public :
		AccessRule() : apduRule(true), nfcRule(true) {}

		inline void setAPDUAccessRule(bool rule) { apduRule = rule; }
		inline void setNFCAccessRule(bool rule) { nfcRule = rule; }

		void addAPDUAccessRule(const ByteArray &apdu,
			const ByteArray &mask);

		inline bool isAuthorizedAccess(void) const
		{
			return (apduRule || (listFilters.size() > 0));
		}
		bool isAuthorizedAPDUAccess(const ByteArray &command) const;
		bool isAuthorizedNFCAccess(void) const;

		friend class AccessCondition;
	};

	class AccessCondition
	{
	private :
		ByteArray aid;
		map<ByteArray, AccessRule> mapRules;

	public :
		AccessCondition() {}

		inline void setAID(const ByteArray &aid) { this->aid = aid; }
		inline const ByteArray getAID() const { return aid; }
		void setAccessCondition(bool rule);
		void addAccessRule(const ByteArray &hash);

		void setAPDUAccessRule(const ByteArray &certHash, bool rule);
		void addAPDUAccessRule(const ByteArray &certHash,
			const ByteArray &apdu, const ByteArray &mask);
		void addAPDUAccessRule(const ByteArray &certHash,
			const ByteArray &rule);

		void setNFCAccessRule(const ByteArray &certHash, bool rule);

		bool isAuthorizedAccess(const ByteArray &certHash) const;
		bool isAuthorizedAPDUAccess(const ByteArray &certHash,
			const ByteArray &command) const;
		bool isAuthorizedNFCAccess(const ByteArray &certHash) const;

		AccessRule *getAccessRule(const ByteArray &certHash);
		const AccessRule *getAccessRule(const ByteArray &certHash) const;

		friend class AccessControlList;
	};

} /* namespace smartcard_service_api */
#endif /* ACCESSCONDITION_H_ */
