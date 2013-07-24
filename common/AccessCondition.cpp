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
#include "AccessCondition.h"

namespace smartcard_service_api
{
	void APDUAccessRule::loadAPDUAccessRule(const ByteArray &data)
	{
		SimpleTLV tlv(data);

		if (tlv.decodeTLV() == true)
		{
			switch (tlv.getTag())
			{
			case 0xA0 : /* CHOICE 0 : APDUPermission */
				permission = SimpleTLV::getBoolean(tlv.getValue());
				break;

			case 0xA1 : /* CHOICE 1 : APDUFilters */
				tlv.enterToValueTLV();
				while (tlv.decodeTLV() == true)
				{
					if (tlv.getTag() == 0x04) /* OCTET STRING */
					{
						ByteArray apdu, mask, value;

						value = tlv.getValue();

						_DBG("APDU rule : %s", value.toString());

						if (value.getLength() == 8) /* apdu 4 bytes + mask 4 bytes */
						{
							apdu.setBuffer(value.getBuffer(), 4);
							mask.setBuffer(value.getBuffer(4), 4);

							pair<ByteArray, ByteArray> newItem(apdu, mask);

							mapApduFilters.insert(newItem);
						}
						else
						{
							_ERR("Invalid APDU rule : %s", value.toString());
						}
					}
					else
					{
						_ERR("Unknown tag : 0x%02X", tlv.getTag());
					}
				}
				tlv.returnToParentTLV();
				break;

			default :
				_ERR("Unknown tag : 0x%02X", tlv.getTag());
				break;
			}
		}
	}

	bool APDUAccessRule::isAuthorizedAccess(const ByteArray &command)
	{
		bool result = false;

		if (mapApduFilters.size() > 0)
		{
			/* TODO : search command and check validity */
		}
		else
		{
			/* no filter entry. if permission is true, all access will be granted, if not, all access will be denied */
			result = permission;
		}

		return result;
	}

	void APDUAccessRule::printAPDUAccessRules()
	{
		_DBG("  +-- APDU Access Rule");

		if (mapApduFilters.size() > 0)
		{
			map<ByteArray, ByteArray>::iterator iterMap;

			for (iterMap = mapApduFilters.begin(); iterMap != mapApduFilters.end(); iterMap++)
			{
				_DBG("  +--- APDU : %s, Mask : %s", ((ByteArray)(iterMap->first)).toString(), iterMap->second.toString());
			}
		}
		else
		{
			_DBG("  +--- permission : %s", permission ? "granted all" : "denied all");
		}
	}

	void NFCAccessRule::loadNFCAccessRule(const ByteArray &data)
	{
		permission = SimpleTLV::getBoolean(data);
	}

	bool NFCAccessRule::isAuthorizedAccess(void)
	{
		bool result = false;

		result = permission;

		return result;
	}

	void NFCAccessRule::printNFCAccessRules()
	{
		_DBG("   +-- NFC Access Rule");
		_DBG("   +--- permission : %s", permission ? "granted all" : "denied all");
	}

	void AccessCondition::loadAccessCondition(ByteArray &aid, ByteArray &data)
	{
		if (data.getLength() > 0)
		{
			SimpleTLV tlv(data);

			while (tlv.decodeTLV() == true && tlv.getTag() == 0x30) /* SEQUENCE */
			{
				if (tlv.getLength() > 0)
				{
					/* access granted for specific applications */
					tlv.enterToValueTLV();
					if (tlv.decodeTLV())
					{
						switch (tlv.getTag())
						{
						case 0x04 : /* OCTET STRING : CertHash */
							_DBG("aid : %s, hash : %s", aid.toString(), tlv.getValue().toString());

							hashes.push_back(tlv.getValue());
							break;

						case 0xA0 : /* CHOICE 0 : AccessRules */
							tlv.enterToValueTLV();
							if (tlv.decodeTLV())
							{
								switch (tlv.getTag())
								{
								case 0xA0 : /* CHOICE 0 : APDUAccessRule */
									apduRule.loadAPDUAccessRule(tlv.getValue());
									break;

								case 0xA1 : /* CHOICE 1 : NFCAccessRule */
									nfcRule.loadNFCAccessRule(tlv.getValue());
									break;

								default :
									_ERR("Unknown tag : 0x%02X", tlv.getTag());
									break;
								}
							}
							else
							{
								_ERR("tlv.decodeTLV failed");
							}
							tlv.returnToParentTLV();
							break;

						default :
							_ERR("Unknown tag : 0x%02X", tlv.getTag());
							break;
						}
					}
					else
					{
						_ERR("tlv.decodeTLV failed");
					}
					tlv.returnToParentTLV();
				}
				else
				{
					_INFO("access granted for all applications, aid : %s", aid.toString());

					permission = true;
					break;
				}
			}
		}
		else
		{
			_INFO("access denied for all applications, aid : %s", aid.toString());

			permission = false;
		}
	}

	bool AccessCondition::isAuthorizedAccess(ByteArray &certHash)
	{
		bool result = false;

		if (hashes.size() > 0)
		{
			size_t i;

			for (i = 0; i < hashes.size(); i++)
			{
				if (certHash == hashes[i])
				{
					result = true;
					break;
				}
			}
		}
		else
		{
			result = permission;
		}

		return result;
	}

	bool AccessCondition::isAuthorizedAPDUAccess(ByteArray &command)
	{
		bool result = false;

		result = apduRule.isAuthorizedAccess(command);

		return result;
	}

	bool AccessCondition::isAuthorizedNFCAccess()
	{
		bool result = false;

		result = nfcRule.isAuthorizedAccess();

		return result;
	}

	void AccessCondition::printAccessConditions()
	{
		_DBG(" +-- Access Condition");

		if (hashes.size() > 0)
		{
			size_t i;

			for (i = 0; i < hashes.size(); i++)
			{
				_DBG(" +--- hash : %s", hashes[i].toString());
			}
		}
		else
		{
			_DBG(" +--- permission : %s", permission ? "granted all" : "denied all");
		}
	}
} /* namespace smartcard_service_api */
