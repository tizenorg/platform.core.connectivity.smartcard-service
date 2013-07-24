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

	bool AccessRule::isAuthorizedAPDUAccess(const ByteArray &command) const
	{
		bool result = false;

		if (command.size() < 4) /* apdu header size */
			return false;

		if (listFilters.size() > 0)
		{
			unsigned int cmd, mask, rule;
			vector<pair<ByteArray, ByteArray> >::const_iterator item;

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

	bool AccessRule::isAuthorizedNFCAccess(void) const
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

	const AccessRule *AccessCondition::getAccessRule(const ByteArray &certHash) const
	{
		const AccessRule *result = NULL;
		map<ByteArray, AccessRule>::const_iterator item;

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

	bool AccessCondition::isAuthorizedAccess(const ByteArray &certHash) const
	{
		bool result = permission;
		const AccessRule *rule = getAccessRule(certHash);

		if (rule != NULL) {
			result = true;
		}

		return result;
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
		if (rule.size() != 8)
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
		const ByteArray &command) const
	{
		bool result = false;
		const AccessRule *rule = getAccessRule(certHash);

		if (rule != NULL) {
			result = rule->isAuthorizedAPDUAccess(command);
		}

		return result;
	}

	bool AccessCondition::isAuthorizedNFCAccess(const ByteArray &certHash) const
	{
		bool result = false;
		const AccessRule *rule = getAccessRule(certHash);

		if (rule != NULL) {
			result = rule->isAuthorizedNFCAccess();
		}

		return result;
	}
} /* namespace smartcard_service_api */
