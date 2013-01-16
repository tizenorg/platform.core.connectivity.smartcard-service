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
#include <vector>
#include <map>

/* SLP library header */

/* local header */
#include "ByteArray.h"

using namespace std;

namespace smartcard_service_api
{
	class APDUAccessRule
	{
	private :
		bool permission;
		map<ByteArray, ByteArray> mapApduFilters;

	public :
		APDUAccessRule()
		{
			permission = true;
		}

		void loadAPDUAccessRule(const ByteArray &data);
		bool isAuthorizedAccess(const ByteArray &command);

		void printAPDUAccessRules();
	};

	class NFCAccessRule
	{
	private :
		bool permission;

	public :
		NFCAccessRule()
		{
			permission = true;
		}

		void loadNFCAccessRule(const ByteArray &data);
		bool isAuthorizedAccess(void);

		void printNFCAccessRules();
	};

	class AccessCondition
	{
	private :
		bool permission;
		ByteArray aid;
		vector<ByteArray> hashes;
		APDUAccessRule apduRule;
		NFCAccessRule nfcRule;

	public :
		AccessCondition() : permission(false)
		{
		}

		void loadAccessCondition(ByteArray &aid, ByteArray &data);
		bool isAuthorizedAccess(ByteArray &certHash);
		bool isAuthorizedAPDUAccess(ByteArray &command);
		bool isAuthorizedNFCAccess();

		void printAccessConditions();
	};

} /* namespace smartcard_service_api */
#endif /* ACCESSCONDITION_H_ */
