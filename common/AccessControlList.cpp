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

namespace smartcard_service_api
{
	const unsigned char all_se_apps[] = { 0x00, 0x00 };
	const unsigned char default_se_app[] = { 0x00, 0x01 };
	const unsigned char all_device_apps[] = { 0x00, 0x02 };

	ByteArray AccessControlList::ALL_SE_APPS(ARRAY_AND_SIZE(all_se_apps));
	ByteArray AccessControlList::DEFAULT_SE_APP(ARRAY_AND_SIZE(default_se_app));
	ByteArray AccessControlList::ALL_DEVICE_APPS(ARRAY_AND_SIZE(all_device_apps));

	AccessControlList::AccessControlList() : allGranted(false)
	{
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

	AccessCondition &AccessControlList::getAccessCondition(const ByteArray &aid)
	{
		map<ByteArray, AccessCondition>::iterator item;

		item = mapConditions.find(aid);
		if (item == mapConditions.end())
		{
			AccessCondition condition;
			pair<ByteArray, AccessCondition> temp(aid, condition);
			mapConditions.insert(temp);

			item = mapConditions.find(aid);
		}

		return item->second;
	}

	const AccessRule *AccessControlList::findAccessRule(const ByteArray &aid,
		const ByteArray &hash) const
	{
		const AccessRule *result = NULL;
		map<ByteArray, AccessCondition>::const_iterator item;

		item = mapConditions.find(aid);
		if (item != mapConditions.end()) {
			result = item->second.getAccessRule(hash);
		}

		return result;
	}

	bool AccessControlList::isAuthorizedAccess(const ByteArray &aid,
		const ByteArray &certHash) const
	{
		vector<ByteArray> hashes;

		hashes.push_back(certHash);

		return isAuthorizedAccess(aid, hashes);
	}

	bool AccessControlList::isAuthorizedAccess(const unsigned char *aidBuffer,
		unsigned int aidLength, const unsigned char *certHashBuffer,
		unsigned int certHashLength) const
	{
		ByteArray aid(aidBuffer, aidLength);
		ByteArray certHash(certHashBuffer, certHashLength);

		return isAuthorizedAccess(aid, certHash);
	}

	bool AccessControlList::isAuthorizedAccess(const ByteArray &aid,
		const vector<ByteArray> &certHashes) const
	{
		return isAuthorizedAccess(aid, certHashes, ByteArray::EMPTY);
	}

	bool AccessControlList::isAuthorizedAccess(const ByteArray &aid,
		const vector<ByteArray> &certHashes, const ByteArray &command) const
	{
		bool result = allGranted;
		vector<ByteArray>::const_reverse_iterator item;
		const AccessRule *rule = NULL;

		if (result == true) {
			goto END;
		}

		/* Step A, find with aid and cert hashes */
		for (item = certHashes.rbegin(); item != certHashes.rend(); item++) {
			rule = findAccessRule(aid, *item);
			if (rule != NULL) {
				if (command.isEmpty()) {
					result = rule->isAuthorizedAccess();
				} else {
					result = rule->isAuthorizedAPDUAccess(command);
				}
				_INFO("rule found (%s): [%s:%s]", result ? "accept" : "deny", aid.toString().c_str(), (*item).toString().c_str());
				goto END;
			}
		}

		/* Step B, find with aid and ALL_DEVICES_APPS */
		rule = findAccessRule(aid, ALL_DEVICE_APPS);
		if (rule != NULL) {
			if (command.isEmpty()) {
				result = rule->isAuthorizedAccess();
			} else {
				result = rule->isAuthorizedAPDUAccess(command);
			}
			_INFO("rule found (%s): [%s:%s]", result ? "accept" : "deny", aid.toString().c_str(), ALL_DEVICE_APPS.toString().c_str());
			goto END;
		}

		/* Step C, find with ALL_SE_APPS and hashes */
		for (item = certHashes.rbegin(); item != certHashes.rend(); item++) {
			rule = findAccessRule(ALL_SE_APPS, *item);
			if (rule != NULL) {
				if (command.isEmpty()) {
					result = rule->isAuthorizedAccess();
				} else {
					result = rule->isAuthorizedAPDUAccess(command);
				}
				_INFO("rule found (%s): [%s:%s]", result ? "accept" : "deny", "All SE Applications", (*item).toString().c_str());
				goto END;
			}
		}

		/* Step D, find with ALL_SE_APPS and ALL_DEVICES_APPS */
		rule = findAccessRule(ALL_SE_APPS, ALL_DEVICE_APPS);
		if (rule != NULL) {
			if (command.isEmpty()) {
				result = rule->isAuthorizedAccess();
			} else {
				result = rule->isAuthorizedAPDUAccess(command);
			}
			_INFO("rule found (%s): [%s:%s]", result ? "accept" : "deny", "All SE Applications", "All device applications");
		}

END :
		return result;
	}

	bool AccessControlList::isAuthorizedNFCAccess(const ByteArray &aid,
		const vector<ByteArray> &certHashes) const
	{
		bool result = allGranted;
		vector<ByteArray>::const_reverse_iterator item;
		const AccessRule *rule = NULL;

		if (result == true) {
			goto END;
		}

		/* Step A, find with aid and cert hashes */
		for (item = certHashes.rbegin(); item != certHashes.rend(); item++) {
			rule = findAccessRule(aid, *item);
			if (rule != NULL) {
				result = rule->isAuthorizedNFCAccess();
				_INFO("rule found (%s): [%s:%s]", result ? "accept" : "deny", aid.toString().c_str(), (*item).toString().c_str());
				goto END;
			}
		}

		/* Step B, find with aid and ALL_DEVICES_APPS */
		rule = findAccessRule(aid, ALL_DEVICE_APPS);
		if (rule != NULL) {
			result = rule->isAuthorizedNFCAccess();
			_INFO("rule found (%s): [%s:%s]", result ? "accept" : "deny", aid.toString().c_str(), "All device applications");
			goto END;
		}

		/* Step C, find with ALL_SE_APPS and hashes */
		for (item = certHashes.rbegin(); item != certHashes.rend(); item++) {
			rule = findAccessRule(ALL_SE_APPS, *item);
			if (rule != NULL) {
				result = rule->isAuthorizedNFCAccess();
				_INFO("rule found (%s): [%s:%s]", result ? "accept" : "deny", "All SE Applications", (*item).toString().c_str());
				goto END;
			}
		}

		/* Step D, find with ALL_SE_APPS and ALL_DEVICES_APPS */
		rule = findAccessRule(ALL_SE_APPS, ALL_DEVICE_APPS);
		if (rule != NULL) {
			result = rule->isAuthorizedNFCAccess();
			_INFO("rule found (%s): [%s:%s]", result ? "accept" : "deny", "All SE Applications", "All device applications");
		}

END :
		return result;
	}

	void AccessControlList::printAccessControlList() const
	{
		ByteArray temp;
		map<ByteArray, AccessCondition>::const_iterator iterMap;

		_DBG("================ Access Control Rules ==================");
		for (iterMap = mapConditions.begin(); iterMap != mapConditions.end(); iterMap++)
		{
			temp = iterMap->first;

			_DBG("+ aid : %s", (temp == DEFAULT_SE_APP) ? "Default Application" : (temp == ALL_SE_APPS) ? "All SE Applications" : temp.toString().c_str());

			iterMap->second.printAccessConditions();
		}
		_DBG("========================================================");
	}

} /* namespace smartcard_service_api */
