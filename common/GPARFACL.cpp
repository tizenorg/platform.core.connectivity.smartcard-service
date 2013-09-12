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
#include "GPARFACL.h"
#include "PKCS15ODF.h"
#include "PKCS15DODF.h"
#include "NumberStream.h"
#include "SimpleTLV.h"
#include "AccessCondition.h"

#ifndef EXTERN_API
#define EXTERN_API __attribute__((visibility("default")))
#endif

namespace smartcard_service_api
{
	static unsigned char oid_globalplatform[] = { 0x2A, 0x86, 0x48, 0x86, 0xFC, 0x6B, 0x81, 0x48, 0x01, 0x01 };
	static ByteArray OID_GLOBALPLATFORM(ARRAY_AND_SIZE(oid_globalplatform));

	GPARFACL::GPARFACL() : AccessControlList()
	{
	}

	GPARFACL::~GPARFACL()
	{
	}

	int GPARFACL::loadACL(Channel *channel)
	{
		int result = SCARD_ERROR_OK;

		_BEGIN();

		if (channel == NULL)
		{
			return SCARD_ERROR_ILLEGAL_PARAM;
		}

		PKCS15 pkcs15(channel);

		/* basically, all requests will be accepted when PKCS #15 doesn't exist or global platform OID is not placed */
		allGranted = false;

		result = pkcs15.select();
		if (result >= SCARD_ERROR_OK)
		{
			PKCS15ODF *odf;

			result = SCARD_ERROR_OK;
			allGranted = true;

			if ((odf = pkcs15.getODF()) != NULL)
			{
				PKCS15DODF *dodf;

				if ((dodf = odf->getDODF()) != NULL)
				{
					result = loadAccessControl(channel, dodf);
					if (result == SCARD_ERROR_OK)
					{
					}
					else
					{
						_INFO("loadAccessControl failed, every request will be accepted.");
						result = SCARD_ERROR_OK;
					}
				}
				else
				{
					_INFO("dodf null, every request will be accepted.");
				}
			}
			else
			{
				_INFO("odf null, every request will be accepted.");
			}
		}
		else
		{
			_ERR("failed to open PKCS15, every request will be denied.");
		}

		_END();

		return result;
	}

	int GPARFACL::loadAccessControl(Channel *channel, PKCS15DODF *dodf)
	{
		int result = -1;
		ByteArray path;

		if ((result = dodf->searchOID(OID_GLOBALPLATFORM, path)) == 0)
		{
			ByteArray data;
			FileObject file(channel);

			_DBG("oid path : %s", path.toString().c_str());

			file.select(NumberStream::getLittleEndianNumber(path));
			file.readBinary(0, 0, file.getFCP()->getFileSize(), data);

			_DBG("data : %s", data.toString().c_str());

			/* PKCS #15 and DODF OID exists. apply access control rule!! */
			allGranted = false;

			SimpleTLV tlv(data);

			if (tlv.decodeTLV() == true && tlv.getTag() == 0x30) /* SEQUENCE : AccessControlMainFile */
			{
				tlv.enterToValueTLV();

				/* refresh Tag */
				ByteArray refreshTag;

				refreshTag = SimpleTLV::getOctetString(tlv);
				_DBG("current refresh tag : %s", refreshTag.toString().c_str());

				if (this->refreshTag != refreshTag) /* need to update access control list */
				{
					this->refreshTag = refreshTag;

					releaseACL();

					/* access control rule path */
					if (tlv.decodeTLV() == true && tlv.getTag() == 0x30) /* SEQUENCE : Path */
					{
						/* TODO : parse path */
						ByteArray path;

						/* OCTET STRING */
						path = SimpleTLV::getOctetString(tlv.getValue());
						_DBG("access control rule path : %s", path.toString().c_str());

						if (loadRules(channel, path) == 0)
						{
							_DBG("loadRules success");
						}
						else
						{
							_ERR("loadRules failed");
						}
					}
				}
				else
				{
					_INFO("access rules are not changed. skip update");
				}
				tlv.returnToParentTLV();
			}
			else
			{
				_ERR("tlv.decodeTLV failed");
			}
		}
		else
		{
			_ERR("OID not found");
		}

		return result;
	}

	int GPARFACL::loadRules(Channel *channel, const ByteArray &path)
	{
		FileObject file(channel);
		ByteArray data, aid;

		file.select(NumberStream::getLittleEndianNumber(path));
		file.readBinary(0, 0, file.getFCP()->getFileSize(), data);

		_DBG("data : %s", data.toString().c_str());

		SimpleTLV tlv(data);

		while (tlv.decodeTLV() == true && tlv.getTag() == 0x30) /* SEQUENCE : Rule */
		{
			tlv.enterToValueTLV();
			if (tlv.decodeTLV() == true)
			{
				/* target */
				switch (tlv.getTag())
				{
				case 0xA0 : /* CHOICE 0 : EXPLICIT AID */
					/* OCTET STRING */
					aid = SimpleTLV::getOctetString(tlv.getValue());
					break;

				case 0x81 : /* CHOICE 1?? : default */
					aid = AccessControlList::DEFAULT_SE_APP;
					break;

				case 0x82 : /* CHOICE 2?? : any application */
					aid = AccessControlList::ALL_SE_APPS;
					break;
				}

				_DBG("aid : %s", aid.toString().c_str());

				/* access condition path */
				if (tlv.decodeTLV() == true && tlv.getTag() == 0x30) /* SEQUENCE : Path */
				{
					ByteArray path;

					/* OCTET STRING */
					path = SimpleTLV::getOctetString(tlv.getValue());
					_DBG("path : %s", path.toString().c_str());

					if (loadAccessConditions(channel, aid, path) == 0)
					{
						_DBG("loadCertHashes success");
					}
					else
					{
						_ERR("loadCertHashes failed");
					}
				}
				else
				{
					_ERR("decodeTLV failed");
				}
			}
			else
			{
				_ERR("decodeTLV failed");
			}
			tlv.returnToParentTLV();
		}

		return 0;
	}

	static void loadAPDUAccessRule(AccessRule *rule, const ByteArray &data)
	{
		SimpleTLV tlv(data);

		if (rule == NULL) {
			_ERR("invalid parameter");
			return;
		}

		if (tlv.decodeTLV() == true)
		{
			switch (tlv.getTag())
			{
			case 0xA0 : /* CHOICE 0 : APDUPermission */
				rule->setAPDUAccessRule(SimpleTLV::getBoolean(tlv.getValue()));
				break;

			case 0xA1 : /* CHOICE 1 : APDUFilters */
				tlv.enterToValueTLV();
				while (tlv.decodeTLV() == true)
				{
					if (tlv.getTag() == 0x04) /* OCTET STRING */
					{
						ByteArray apdu, mask, value;

						value = tlv.getValue();

						_DBG("APDU rule : %s", value.toString().c_str());

						if (value.size() == 8) /* apdu 4 bytes + mask 4 bytes */
						{
							apdu.assign(value.getBuffer(), 4);
							mask.assign(value.getBuffer(4), 4);

							rule->addAPDUAccessRule(apdu, mask);
						}
						else
						{
							_ERR("Invalid APDU rule : %s", value.toString().c_str());
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

	static void loadNFCAccessRule(AccessRule *rule, const ByteArray &data)
	{
		if (rule == NULL) {
			_ERR("invalid parameter");
			return;
		}

		rule->setNFCAccessRule(SimpleTLV::getBoolean(data));
	}

	static void loadAccessCondition(AccessCondition &condition, const ByteArray &data)
	{
		if (data.size() > 0)
		{
			SimpleTLV tlv(data);
			ByteArray hash;

			while (tlv.decodeTLV() == true && tlv.getTag() == 0x30) /* SEQUENCE */
			{
				if (tlv.size() > 0)
				{
					/* access granted for specific applications */
					tlv.enterToValueTLV();
					if (tlv.decodeTLV())
					{
						switch (tlv.getTag())
						{
						case 0x04 : /* OCTET STRING : CertHash */
							_DBG("aid : %s, hash : %s", condition.getAID().toString().c_str(), tlv.getValue().toString().c_str());

							hash = tlv.getValue();
							condition.addAccessRule(tlv.getValue());
							break;

						case 0xA0 : /* CHOICE 0 : AccessRules */
							tlv.enterToValueTLV();
							if (tlv.decodeTLV())
							{
								AccessRule *rule = condition.getAccessRule(hash);
								if (rule == NULL) {
									condition.addAccessRule(hash);
									rule = condition.getAccessRule(hash);
								}

								switch (tlv.getTag())
								{
								case 0xA0 : /* CHOICE 0 : APDUAccessRule */
									loadAPDUAccessRule(rule, tlv.getValue());
									break;

								case 0xA1 : /* CHOICE 1 : NFCAccessRule */
									loadNFCAccessRule(rule, tlv.getValue());
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
					_INFO("access denied for all applications, aid : %s", condition.getAID().toString().c_str());

					condition.setAccessCondition(false);
					break;
				}
			}
		}
		else
		{
			_INFO("access denied for all applications, aid : %s", condition.getAID().toString().c_str());

			condition.setAccessCondition(false);
		}
	}

	int GPARFACL::loadAccessConditions(Channel *channel, const ByteArray &aid, const ByteArray &path)
	{
		FileObject file(channel);
		ByteArray data;

		file.select(NumberStream::getLittleEndianNumber(path));
		file.readBinary(0, 0, file.getFCP()->getFileSize(), data);

		_DBG("data : %s", data.toString().c_str());

		AccessCondition condition;

		condition.setAID(aid);
		loadAccessCondition(condition, data);

		pair<ByteArray, AccessCondition> newItem(aid, condition);

		mapConditions.insert(newItem);

		return 0;
	}

} /* namespace smartcard_service_api */
