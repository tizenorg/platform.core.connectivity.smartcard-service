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

/* SLP library header */

/* local header */
#include "Debug.h"
#include "GPARAACL.h"
#include "GPARAM.h"
#include "NumberStream.h"
#include "SimpleTLV.h"
#include "ISO7816BERTLV.h"
#include "AccessCondition.h"

#ifndef EXTERN_API
#define EXTERN_API __attribute__((visibility("default")))
#endif

namespace smartcard_service_api
{
	static unsigned char aid_aram[] = { 0xA0, 0x00, 0x00, 0x01, 0x51, 0x41, 0x43, 0x4C, 0x00 };
	static ByteArray AID_ARAM(ARRAY_AND_SIZE(aid_aram));

#define GET_DATA_ALL		0
#define GET_DATA_SPECIFIC	1
#define GET_DATA_REFRESH_TAG	2
#define GET_DATA_NEXT		3

#define ARAM_TAG_ALL_AR		0x0000FF40
#define ARAM_TAG_AR		0x0000FF50
#define ARAM_TAG_REFRESH	0x0000DF20

#define DO_TAG_AID_REF		0x0000004F
#define DO_TAG_AID_REF_DEFAULT	0x000000C0
#define DO_TAG_HASH_REF		0x000000C1
#define DO_TAG_APDU_AR		0x000000D0
#define DO_TAG_NFC_AR		0x000000D1
#define DO_TAG_REF		0x000000E1
#define DO_TAG_REF_AR		0x000000E2
#define DO_TAG_AR		0x000000E3

	GPARAACL::GPARAACL() : AccessControlList()
	{
	}

	GPARAACL::~GPARAACL()
	{
	}

	static ByteArray getAID(SimpleTLV &tlv)
	{
		ByteArray result;

		_BEGIN();

		if (tlv.decodeTLV() == true) {
			switch (tlv.getTag()) {
			case DO_TAG_AID_REF :
				if (tlv.size() > 0) {
					result = tlv.getValue();
				} else {
					result = AccessControlList::ALL_SE_APPS;
				}
				break;

			case DO_TAG_AID_REF_DEFAULT :
				result = AccessControlList::DEFAULT_SE_APP;
				break;

			default :
				_ERR("decodeTLV failed, %s", tlv.toString().c_str());
				break;
			}
		} else {
			_ERR("decodeTLV failed, %s", tlv.toString().c_str());
		}

		_END();

		return result;
	}

	static ByteArray getHash(SimpleTLV &tlv)
	{
		ByteArray result;

		_BEGIN();

		if (tlv.decodeTLV() == true &&
			tlv.getTag() == DO_TAG_HASH_REF) {
			if (tlv.size() > 0) {
				result = tlv.getValue();
			} else {
				result = AccessControlList::ALL_DEVICE_APPS;
			}
		} else {
			_ERR("decodeTLV failed, %s", tlv.toString().c_str());
		}

		_END();

		return result;
	}

	static int parseRefDO(SimpleTLV &tlv, ByteArray &aid, ByteArray &hash)
	{
		int result = SCARD_ERROR_OK;

		_BEGIN();

		if (tlv.decodeTLV() == true && tlv.getTag() == DO_TAG_REF) {
			tlv.enterToValueTLV();
			aid = getAID(tlv);
			hash = getHash(tlv);
			tlv.returnToParentTLV();

			_DBG("aid : %s, hash : %s", aid.toString().c_str(), hash.toString().c_str());
		} else {
			_ERR("unknown tag : %s", tlv.toString().c_str());
			result = SCARD_ERROR_ILLEGAL_PARAM;
		}

		_END();

		return result;
	}

	static int parseARDO(SimpleTLV &tlv, vector<ByteArray> &apduRule,
		ByteArray &nfcRule)
	{
		int result = SCARD_ERROR_OK;

		_BEGIN();

		if (tlv.decodeTLV() == true && tlv.getTag() == DO_TAG_AR) {
			tlv.enterToValueTLV();
			while (tlv.decodeTLV() == true) {
				int length = tlv.size();

				switch (tlv.getTag()) {
				case DO_TAG_APDU_AR :
					if (length > 1) {
						int i;
						ByteArray temp;

						for (i = 0; i < length; i += 8) {
							temp.assign(tlv.getValue().getBuffer(i), 8);
							_DBG("apdu rule[%d] : %s", temp.size(), temp.toString().c_str());
							apduRule.push_back(temp);
						}
					} else if (length == 1){
						_DBG("apdu rule : %s", tlv.getValue().toString().c_str());
						apduRule.push_back(tlv.getValue());
					} else {
						_ERR("invalid rule, %s", tlv.toString().c_str());
					}
					break;

				case DO_TAG_NFC_AR :
					nfcRule = tlv.getValue();
					_DBG("nfc rule : %s", tlv.getValue().toString().c_str());
					break;

				default :
					break;
				}
			}
			tlv.returnToParentTLV();
		} else {
			result = SCARD_ERROR_ILLEGAL_PARAM;
		}

		_END();

		return result;
	}

	void GPARAACL::addCondition(const ByteArray &aid, const ByteArray &hash,
		const vector<ByteArray> &apduRule, const ByteArray &nfcRule)
	{
		AccessCondition &condition = getAccessCondition(aid);

		_BEGIN();

		condition.addAccessRule(hash);

		if (apduRule.size() > 0) {
			if (apduRule.size() == 1 &&
				apduRule[0].size() == 1) {
				/* apdu grant/deny */
				if (apduRule[0][0] == 1) {
					condition.setAPDUAccessRule(hash, true);
				} else {
					condition.setAPDUAccessRule(hash, false);
				}
			} else {
				size_t i;

				for (i = 0; i < apduRule.size(); i++) {
					condition.addAPDUAccessRule(hash, apduRule[i]);
				}
			}
		}

		if (nfcRule.size() == 1) {
			if (nfcRule[0] == 1) {
				condition.setNFCAccessRule(hash, true);
			} else {
				condition.setNFCAccessRule(hash, false);
			}
		}

		_END();
	}

	int GPARAACL::updateRule(const ByteArray &data)
	{
		int result = SCARD_ERROR_OK;
		SimpleTLV tlv(data);

		_BEGIN();

		while (tlv.decodeTLV() == true) {
			if (tlv.getTag() == DO_TAG_REF_AR) {
				ByteArray aid, hash, nfcRule;
				vector<ByteArray> apduRule;

				tlv.enterToValueTLV();
				result = parseRefDO(tlv, aid, hash);

				if (result >= SCARD_ERROR_OK) {
					result = parseARDO(tlv, apduRule, nfcRule);
				}
				tlv.returnToParentTLV();

				addCondition(aid, hash, apduRule, nfcRule);
			} else {
				_ERR("unknown tag, [%x]", tlv.getTag());
				result = SCARD_ERROR_ILLEGAL_PARAM;
				break;
			}
		}

		_END();

		return result;
	}

	int GPARAACL::loadACL(GPARAM &aram)
	{
		int result = SCARD_ERROR_OK;
		ByteArray refreshTag, response;

		_BEGIN();

		if (aram.isClosed() == true) {
			return SCARD_ERROR_ILLEGAL_STATE;
		}

		/* get refresh tag */
		result = aram.getDataRefreshTag(refreshTag);
		if (result >= SCARD_ERROR_OK) {
			/* check refresh tag */
			if (this->refreshTag.isEmpty() == true ||
				this->refreshTag != refreshTag) {
				result = aram.getDataAll(response);
				if (result >= SCARD_ERROR_OK) {
					result = updateRule(response);

					/* update refresh tag */
					this->refreshTag = refreshTag;
				} else {
					_ERR("getDataAll failed, [%x]", result);
				}
			}
			else
			{
				_INFO("access rules are not changed. skip update");
			}
		} else {
			_ERR("transmitSync failed, %x", result);
		}

		_END();

		return result;
	}

	int GPARAACL::loadACL(Channel *channel)
	{
		int result = SCARD_ERROR_OK;

		_BEGIN();

		if (channel == NULL) {
			return SCARD_ERROR_ILLEGAL_PARAM;
		}

		GPARAM aram(channel);

		result = aram.select();
		if (result >= SCARD_ERROR_OK) {
			result = loadACL(aram);
		} else {
			_ERR("select failed, [%x]", result);
		}

		_END();

		return result;
	}

	static bool _isAuthorizedAccess(const ByteArray &data, const ByteArray &command)
	{
		vector<ByteArray> apduRule;
		ByteArray nfcRule;
		SimpleTLV tlv(data);
		bool result = false;

		if (parseARDO(tlv, apduRule, nfcRule) >= SCARD_ERROR_OK) {
			if (apduRule.size() > 0) {
				if (apduRule.size() > 1 ||
					apduRule[0].size() != 1) {
					if (command.size() > 0) {
						/* TODO : check apdu rule */
					} else {
						/* check hash only */
						result = true;
					}
				} else {
					result = (apduRule[0][0] == 1 ? true : false);
				}
			} else {
				_ERR("unknown data : %s", tlv.toString().c_str());
			}
		} else {
			_ERR("parseARDO failed : %s", tlv.toString().c_str());
		}

		return result;
	}

	static bool _isAuthorizedNFCAccess(const ByteArray &data)
	{
		vector<ByteArray> apduRule;
		ByteArray nfcRule;
		SimpleTLV tlv(data);
		bool result = false;

		if (parseARDO(tlv, apduRule, nfcRule) >= SCARD_ERROR_OK) {
			if (nfcRule.size() == 1) {
				result = (nfcRule[0] == 1 ? true : false);
			} else {
				_ERR("unknown data : %s", nfcRule.toString().c_str());
			}
		} else {
			_ERR("parseARDO failed : %s", tlv.toString().c_str());
		}

		return result;
	}

	bool GPARAACL::isAuthorizedAccess(GPARAM &aram, const ByteArray &aid,
		const ByteArray &certHash) const
	{
		vector<ByteArray> hashes;

		hashes.push_back(certHash);

		return isAuthorizedAccess(aram, aid, hashes, ByteArray::EMPTY);
	}

	bool GPARAACL::isAuthorizedAccess(GPARAM &aram,
		const unsigned char *aidBuffer,
		unsigned int aidLength,
		const unsigned char *certHashBuffer,
		unsigned int certHashLength) const
	{
		ByteArray aid(aidBuffer, aidLength);
		ByteArray hash(certHashBuffer, certHashLength);

		return isAuthorizedAccess(aram, aid, hash);
	}

	bool GPARAACL::isAuthorizedAccess(GPARAM &aram, const ByteArray &aid,
		const vector<ByteArray> &certHashes) const
	{
		return isAuthorizedAccess(aram, aid, certHashes, ByteArray::EMPTY);
	}

	bool GPARAACL::isAuthorizedAccess(GPARAM &aram, const ByteArray &aid,
		const vector<ByteArray> &certHashes, const ByteArray &command) const
	{
		bool result = allGranted;
		ByteArray data;
		vector<ByteArray>::const_reverse_iterator item;

		if (aram.isClosed() == true)
			return result;

		_BEGIN();

		if (result == true) {
			goto END;
		}
		/* Step A, find with aid and cert hashes */
		for (item = certHashes.rbegin();
			result == false && item != certHashes.rend();
			item++) {
			if (aram.getDataSpecific(aid, *item, data)
				>= SCARD_ERROR_OK && data.size() > 0) {
				result = _isAuthorizedAccess(data, command);
				_INFO("rule found (%s): [%s:%s]", result ? "accept" : "deny", aid.toString().c_str(), (*item).toString().c_str());
				goto END;
			}
		}

		/* Step B, find with aid and ALL_DEVICES_APPS */
		if (aram.getDataSpecific(aid, ByteArray::EMPTY, data)
			>= SCARD_ERROR_OK && data.size() > 0) {
			result = _isAuthorizedAccess(data, command);
			_INFO("rule found (%s): [%s:%s]", result ? "accept" : "deny", aid.toString().c_str(), "All device applications");
			goto END;
		}

		/* Step C, find with ALL_SE_APPS and hashes */
		for (item = certHashes.rbegin();
			result == false && item != certHashes.rend();
			item++) {
			if (aram.getDataSpecific(ByteArray::EMPTY, *item, data)
				>= SCARD_ERROR_OK && data.size() > 0) {
				result = _isAuthorizedAccess(data, command);
				_INFO("rule found (%s): [%s:%s]", result ? "accept" : "deny", "All SE Applications", (*item).toString().c_str());
				goto END;
			}
		}

		/* Step D, find with ALL_SE_APPS and ALL_DEVICES_APPS */
		if (aram.getDataSpecific(ByteArray::EMPTY, ByteArray::EMPTY, data)
			>= SCARD_ERROR_OK && data.size() > 0) {
			result = _isAuthorizedAccess(data, command);
			_INFO("rule found (%s): [%s:%s]", result ? "accept" : "deny", "All SE Applications", "All device applications");
			goto END;
		}

	END :
		_END();

		return result;
	}

	bool GPARAACL::isAuthorizedNFCAccess(GPARAM &aram, const ByteArray &aid,
		const vector<ByteArray> &certHashes) const
	{
		bool result = allGranted;
		ByteArray data;
		vector<ByteArray>::const_reverse_iterator item;

		if (aram.isClosed() == true)
			return result;

		_BEGIN();

		if (result == true) {
			goto END;
		}
		/* Step A, find with aid and cert hashes */
		for (item = certHashes.rbegin();
			result == false && item != certHashes.rend();
			item++) {
			if (aram.getDataSpecific(aid, *item, data)
				>= SCARD_ERROR_OK && data.size() > 0) {
				result = _isAuthorizedNFCAccess(data);
				_INFO("rule found (%s): [%s:%s]", result ? "accept" : "deny", aid.toString().c_str(), (*item).toString().c_str());
				goto END;
			}
		}

		/* Step B, find with aid and ALL_DEVICES_APPS */
		if (aram.getDataSpecific(aid, ByteArray::EMPTY, data)
			>= SCARD_ERROR_OK && data.size() > 0) {
			result = _isAuthorizedNFCAccess(data);
			_INFO("rule found (%s): [%s:%s]", result ? "accept" : "deny", aid.toString().c_str(), "All device applications");
			goto END;
		}

		/* Step C, find with ALL_SE_APPS and hashes */
		for (item = certHashes.rbegin();
			result == false && item != certHashes.rend();
			item++) {
			if (aram.getDataSpecific(ByteArray::EMPTY, *item, data)
				>= SCARD_ERROR_OK && data.size() > 0) {
				result = _isAuthorizedNFCAccess(data);
				_INFO("rule found (%s): [%s:%s]", result ? "accept" : "deny", "All SE Applications", (*item).toString().c_str());
				goto END;
			}
		}

		/* Step D, find with ALL_SE_APPS and ALL_DEVICES_APPS */
		if (aram.getDataSpecific(ByteArray::EMPTY, ByteArray::EMPTY, data)
			>= SCARD_ERROR_OK && data.size() > 0) {
			result = _isAuthorizedNFCAccess(data);
			_INFO("rule found (%s): [%s:%s]", result ? "accept" : "deny", "All SE Applications", "All device applications");
			goto END;
		}

	END :
		_END();

		return result;
	}
} /* namespace smartcard_service_api */
