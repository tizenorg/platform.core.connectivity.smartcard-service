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

/* standard library header */
#include <map>
#include <vector>

/* SLP library header */

/* local header */
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

		void printAccessRules();

	public :
		AccessRule() : apduRule(true), nfcRule(true)
		{
		}

		inline void setAPDUAccessRule(bool rule) { apduRule = rule; }
		inline void setNFCAccessRule(bool rule) { nfcRule = rule; }

		void addAPDUAccessRule(const ByteArray &apdu,
			const ByteArray &mask);

		inline bool isAuthorizedAccess(void)
		{
			return (apduRule || (listFilters.size() > 0));
		}
		bool isAuthorizedAPDUAccess(const ByteArray &command);
		bool isAuthorizedNFCAccess(void);

		friend class AccessCondition;
	};

	class AccessCondition
	{
	private :
		bool permission;
		ByteArray aid;
		map<ByteArray, AccessRule> mapRules;

		void printAccessConditions();

	public :
		AccessCondition() : permission(false)
		{
		}

		inline void setAID(const ByteArray &aid) { this->aid = aid; }
		inline ByteArray getAID() { return aid; }
		inline void setAccessCondition(bool rule) { permission = rule; }
		void addAccessRule(const ByteArray &hash);
		AccessCondition *getAccessCondition(const ByteArray &hash);

		void setAPDUAccessRule(const ByteArray &certHash, bool rule);
		void addAPDUAccessRule(const ByteArray &certHash,
			const ByteArray &apdu, const ByteArray &mask);
		void addAPDUAccessRule(const ByteArray &certHash,
			const ByteArray &rule);

		void setNFCAccessRule(const ByteArray &certHash, bool rule);

		bool isAuthorizedAccess(const ByteArray &certHash);
		bool isAuthorizedAPDUAccess(const ByteArray &certHash,
			const ByteArray &command);
		bool isAuthorizedNFCAccess(const ByteArray &certHash);

		AccessRule *getAccessRule(const ByteArray &certHash);

		friend class AccessControlList;
	};

} /* namespace smartcard_service_api */
#endif /* ACCESSCONDITION_H_ */
