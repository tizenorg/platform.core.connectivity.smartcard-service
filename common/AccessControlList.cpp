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

/* standard library header */
#include <stdio.h>

/* SLP library header */

/* local header */
#include "Debug.h"
#include "AccessControlList.h"
#include "PKCS15.h"
#include "AccessCondition.h"

namespace smartcard_service_api
{
	const unsigned char aid_all[] = { 0x00, 0x00 };
	const unsigned char aid_default[] = { 0x00, 0x01 };

	ByteArray AccessControlList::AID_ALL(ARRAY_AND_SIZE(aid_all));
	ByteArray AccessControlList::AID_DEFAULT(ARRAY_AND_SIZE(aid_default));

	AccessControlList::AccessControlList()
	{
		allGranted = false;
	}

	AccessControlList::~AccessControlList()
	{
		releaseACL();
	}

	void AccessControlList::releaseACL()
	{
		mapConditions.clear();
		allGranted = false;
	}

	bool AccessControlList::isAuthorizedAccess(ByteArray aid, ByteArray certHash, bool update)
	{
		bool result = allGranted;
		map<ByteArray, AccessCondition>::iterator iterMap;

		if (result == false)
		{
			SCARD_DEBUG("aid : %s", aid.toString());
			SCARD_DEBUG("hash : %s", certHash.toString());

			/* null aid means default applet */
			if (aid.isEmpty() == true)
			{
				aid = AID_DEFAULT;
			}

			/* first.. find hashes matched with aid */
			if ((iterMap = mapConditions.find(aid)) != mapConditions.end())
			{
				result = iterMap->second.isAuthorizedAccess(certHash);
			}
			/* finally.. find hashes in 'all' list */
			else if ((iterMap = mapConditions.find(AID_ALL)) != mapConditions.end())
			{
				result = iterMap->second.isAuthorizedAccess(certHash);
			}
		}

		return result;
	}

	bool AccessControlList::isAuthorizedAccess(ByteArray aid, ByteArray certHash)
	{
		return isAuthorizedAccess(aid, certHash, true);
	}

	bool AccessControlList::isAuthorizedAccess(unsigned char *aidBuffer, unsigned int aidLength, unsigned char *certHashBuffer, unsigned int certHashLength)
	{
		return isAuthorizedAccess(ByteArray(aidBuffer, aidLength), ByteArray(certHashBuffer, certHashLength), true);
	}

	bool AccessControlList::isAuthorizedAccess(ByteArray aid, vector<ByteArray> &certHashes)
	{
		bool result;
		size_t i;

		result = allGranted;

		if (result == false)
		{
			for (i = 0; result == false && i < certHashes.size(); i++)
			{
				result = isAuthorizedAccess(aid, certHashes[i], false);
			}
		}

		return result;
	}

	void AccessControlList::printAccessControlList()
	{
		ByteArray temp;

		/* release map and vector */
		map<ByteArray, AccessCondition>::iterator iterMap;

		SCARD_DEBUG("================ Certification Hashes ==================");
		for (iterMap = mapConditions.begin(); iterMap != mapConditions.end(); iterMap++)
		{
			temp = iterMap->first;

			SCARD_DEBUG("+ aid : %s", (temp == AID_DEFAULT) ? "DEFAULT" : (temp == AID_ALL) ? "ALL" : temp.toString());

			iterMap->second.printAccessConditions();
		}
		SCARD_DEBUG("========================================================");
	}

} /* namespace smartcard_service_api */
