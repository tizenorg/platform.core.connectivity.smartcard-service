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
#include "SimpleTLV.h"
#include "AccessControlList.h"
#include "AccessCondition.h"

namespace smartcard_service_api
{
	void AccessRule::addAPDUAccessRule(const ByteArray &apdu,
		const ByteArray &mask)
	{
		pair<ByteArray, ByteArray> item(apdu, mask);

		listFilters.push_back(item);
	}

	bool AccessRule::isAuthorizedAPDUAccess(const ByteArray &command)
	{
		bool result = false;

		if (command.getLength() < 4) /* apdu header size */
			return false;

		if (listFilters.size() > 0)
		{
			unsigned int cmd, mask, rule;
			vector<pair<ByteArray, ByteArray> >::iterator item;

			cmd = *(unsigned int *)command.getBuffer();
			for (item = listFilters.begin(); item != listFilters.end(); item++)
			{
				mask = *(unsigned int *)item->second.getBuffer();
				rule = *(unsigned int *)item->first.getBuffer();

				if ((cmd & mask) == rule)
				{
					result = true;
					break;
				}
			}
		}
		else
		{
			/* no filter entry. if permission is true, all access will be granted, if not, all access will be denied */
			result = apduRule;
		}

		return result;
	}

	void AccessRule::printAccessRules()
	{
		if (listFilters.size() > 0)
		{
			vector<pair<ByteArray, ByteArray> >::iterator item;

			_DBG("        +---- Granted APDUs");

			for (item = listFilters.begin(); item != listFilters.end(); item++)
			{
				_DBG("        +----- APDU : %s, Mask : %s", item->first.toString(), item->second.toString());
			}
		}
		else
		{
			_DBG("        +---- APDU Access ALLOW : %s", apduRule ? "ALWAYS" : "NEVER");
		}

		_DBG("        +---- NFC  Access ALLOW : %s", nfcRule ? "ALWAYS" : "NEVER");
	}

	bool AccessRule::isAuthorizedNFCAccess(void)
	{
		return nfcRule;
	}

	AccessRule *AccessCondition::getAccessRule(const ByteArray &certHash)
	{
		AccessRule *result = NULL;
		map<ByteArray, AccessRule>::iterator item;

		item = mapRules.find(certHash);
		if (item != mapRules.end()) {
			result = &item->second;
		}

		return result;
	}

	void AccessCondition::addAccessRule(const ByteArray &hash)
	{
		AccessRule rule;

		pair<ByteArray, AccessRule> item(hash, rule);

		mapRules.insert(item);
	}

	bool AccessCondition::isAuthorizedAccess(const ByteArray &certHash)
	{
		bool result = false;
		map<ByteArray, AccessRule>::iterator item;

		item = mapRules.find(certHash);
		if (item != mapRules.end())
		{
			result = true;
		}
		else
		{
			/* TODO */
			result = permission;
		}

		return result;
	}

	void AccessCondition::printAccessConditions()
	{
		_DBG("   +-- Access Condition");

		if (mapRules.size() > 0)
		{
			map<ByteArray, AccessRule>::iterator item;

			for (item = mapRules.begin(); item != mapRules.end(); item++)
			{
				ByteArray temp = item->first;

				_DBG("   +--- hash : %s", (temp == AccessControlList::ALL_DEVICE_APPS) ? "All device applications" : temp.toString());
				item->second.printAccessRules();
			}
		}
		else
		{
			_DBG("   +--- permission : %s", permission ? "granted all" : "denied all");
		}
	}

	void AccessCondition::setAPDUAccessRule(const ByteArray &certHash,
		bool rule)
	{
		AccessRule *access = getAccessRule(certHash);

		if (access != NULL) {
			access->setAPDUAccessRule(rule);
		}
	}

	void AccessCondition::addAPDUAccessRule(const ByteArray &certHash,
		const ByteArray &apdu, const ByteArray &mask)
	{
		AccessRule *access = getAccessRule(certHash);

		if (access != NULL) {
			access->addAPDUAccessRule(apdu, mask);
		}
	}

	void AccessCondition::addAPDUAccessRule(const ByteArray &certHash,
		const ByteArray &rule)
	{
		if (rule.getLength() != 8)
			return;

		addAPDUAccessRule(certHash, rule.sub(0, 4), rule.sub(4, 4));
	}

	void AccessCondition::setNFCAccessRule(const ByteArray &certHash,
			bool rule)
	{
		AccessRule *access = getAccessRule(certHash);

		if (access != NULL) {
			access->setNFCAccessRule(rule);
		}
	}

	bool AccessCondition::isAuthorizedAPDUAccess(const ByteArray &certHash,
		const ByteArray &command)
	{
		bool result = false;
		AccessRule *access = getAccessRule(certHash);

		if (access != NULL) {
			result = access->isAuthorizedAPDUAccess(command);
		}

		return result;
	}

	bool AccessCondition::isAuthorizedNFCAccess(const ByteArray &certHash)
	{
		bool result = false;
		AccessRule *access = getAccessRule(certHash);

		if (access != NULL) {
			result = access->isAuthorizedNFCAccess();
		}

		return result;
	}
} /* namespace smartcard_service_api */
