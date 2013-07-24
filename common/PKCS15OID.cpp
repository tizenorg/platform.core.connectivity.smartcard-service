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
	PKCS15OID::PKCS15OID(const ByteArray &data)
	{
		parseOID(data);
	}

	PKCS15OID::~PKCS15OID()
	{
	}

	bool PKCS15OID::parseOID(const ByteArray &data)
	{
		bool result = false;
		SimpleTLV tlv(data);

		_BEGIN();

		while (tlv.decodeTLV() == true)
		{
			switch (tlv.getTag())
			{
			case PKCS15::TAG_SEQUENCE :
				if (tlv.size() > 0)
				{
					/* common object attribute */
					tlv.enterToValueTLV();
					if (tlv.decodeTLV() == true && tlv.getTag() == 0x0C) /* ?? */
					{
						name = tlv.getValue();
					}
					tlv.returnToParentTLV();
				}
				else
				{
					/* common object attribute */
					/* if you want to use this value, add member variable and parse here */
//					_ERR("common object attribute is empty");
				}
				break;

			case 0xA0 : /* CHOICE 0 : External Oid??? */
				_ERR("oid doesn't exist");
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

						_DBG("oid : %s", oid.toString().c_str());
					}
					else
					{
						_ERR("oid is empty");
					}

					/* path */
					if (tlv.decodeTLV() == true && tlv.getTag() == PKCS15::TAG_SEQUENCE)
					{
						path = SimpleTLV::getOctetString(tlv.getValue());

						_DBG("path : %s", path.toString().c_str());

						result = true;
					}
					else
					{
						_ERR("sequence is empty");
					}

					tlv.returnToParentTLV();
				}
				else
				{
					_ERR("common dataobject attribute is empty");
				}
				tlv.returnToParentTLV();

				break;

			default :
				_ERR("Unknown tag : 0x%02X", tlv.getTag());
				break;
			}
		}

		_END();

		return result;
	}
} /* namespace smartcard_service_api */
