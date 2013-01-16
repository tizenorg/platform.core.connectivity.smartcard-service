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
#include "PKCS15.h"
#include "PKCS15OID.h"
#include "SimpleTLV.h"

namespace smartcard_service_api
{
	PKCS15OID::PKCS15OID(ByteArray data)
	{
		parseOID(data);
	}

	PKCS15OID::~PKCS15OID()
	{
	}

	bool PKCS15OID::parseOID(ByteArray data)
	{
		bool result = false;
		SimpleTLV tlv(data);

		SCARD_BEGIN();

		while (tlv.decodeTLV() == true)
		{
			switch (tlv.getTag())
			{
			case PKCS15::TAG_SEQUENCE :
				if (tlv.getLength() > 0)
				{
					/* common object attribute */
					tlv.enterToValueTLV();
					if (tlv.decodeTLV() == true && tlv.getTag() == 0x0C) /* ?? */
					{
						name = tlv.getValue();
						SCARD_DEBUG("name : %s", name.toString());
					}
					tlv.returnToParentTLV();
				}
				else
				{
					/* common object attribute */
					/* if you want to use this value, add member variable and parse here */
//					SCARD_DEBUG_ERR("common object attribute is empty");
				}
				break;

			case 0xA0 : /* CHOICE 0 : External Oid??? */
				SCARD_DEBUG_ERR("oid doesn't exist");
				break;

			case 0xA1 : /* CHOICE 1 : OidDO */
				tlv.enterToValueTLV();

				/* attribute */
				if (tlv.decodeTLV() == true && tlv.getTag() == PKCS15::TAG_SEQUENCE)
				{
					tlv.enterToValueTLV();

					/* oid */
					if (tlv.decodeTLV() == true && tlv.getTag() == (unsigned int)0x06) /* ?? */
					{
						oid = tlv.getValue();

						SCARD_DEBUG("oid : %s", oid.toString());
					}
					else
					{
						SCARD_DEBUG_ERR("oid is empty");
					}

					/* path */
					if (tlv.decodeTLV() == true && tlv.getTag() == PKCS15::TAG_SEQUENCE)
					{
						path = SimpleTLV::getOctetString(tlv.getValue());

						SCARD_DEBUG("path : %s", path.toString());

						result = true;
					}
					else
					{
						SCARD_DEBUG_ERR("sequence is empty");
					}

					tlv.returnToParentTLV();
				}
				else
				{
					SCARD_DEBUG_ERR("common dataobject attribute is empty");
				}
				tlv.returnToParentTLV();

				break;

			default :
				SCARD_DEBUG_ERR("Unknown tag : 0x%02X", tlv.getTag());
				break;
			}
		}

		SCARD_END();

		return result;
	}

	ByteArray PKCS15OID::getOID()
	{
		return oid;
	}

	ByteArray PKCS15OID::getName()
	{
		return name;
	}

	ByteArray PKCS15OID::getPath()
	{
		return path;
	}

} /* namespace smartcard_service_api */
