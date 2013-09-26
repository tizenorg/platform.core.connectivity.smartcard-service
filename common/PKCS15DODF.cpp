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

#include "Debug.h"
#include "PKCS15DODF.h"
#include "SimpleTLV.h"

namespace smartcard_service_api
{
	PKCS15DODF::PKCS15DODF(unsigned int fid, Channel *channel) :
		PKCS15Object(channel)
	{
		int ret = 0;

		if ((ret = select(fid)) >= SCARD_ERROR_OK)
		{
			ByteArray dodfData, extra;

			_DBG("response : %s", selectResponse.toString().c_str());

			if ((ret = readBinary(0, 0, getFCP()->getFileSize(), dodfData)) == 0)
			{
				_DBG("odfData : %s", dodfData.toString().c_str());

				parseData(dodfData);
			}
			else
			{
				_ERR("readBinary failed, [%d]", ret);
			}
		}
		else
		{
			_ERR("select failed, [%d]", ret);
		}
	}

	PKCS15DODF::PKCS15DODF(const ByteArray &path, Channel *channel) :
		PKCS15Object(channel)
	{
		int ret = 0;

		if ((ret = select(path)) >= SCARD_ERROR_OK)
		{
			ByteArray dodfData, extra;

			_DBG("response : %s", selectResponse.toString().c_str());

			if ((ret = readBinary(0, 0, getFCP()->getFileSize(), dodfData)) == 0)
			{
				_DBG("dodfData : %s", dodfData.toString().c_str());

				parseData(dodfData);
			}
			else
			{
				_ERR("readBinary failed, [%d]", ret);
			}
		}
		else
		{
			_ERR("select failed, [%d]", ret);
		}
	}

	PKCS15DODF::~PKCS15DODF()
	{
	}

	bool PKCS15DODF::parseData(const ByteArray &data)
	{
		bool result = false;
		SimpleTLV tlv(data);

		while (tlv.decodeTLV())
		{
			switch (tlv.getTag())
			{
			case (unsigned int)0xA1 : /* CHOICE 1 : OidDO */
				{
					PKCS15OID oid(tlv.getValue());

					_DBG("OID DataObject : %s", oid.getOID().toString().c_str());

					mapOID.insert(make_pair(oid.getOID(), oid));
				}
				break;

			default :
				_DBG("Unknown tlv : t [%X], l [%d], v %s", tlv.getTag(), tlv.size(), tlv.getValue().toString().c_str());
				break;
			}
		}

		_INFO("dataList.size() = %d", mapOID.size());

		return result;
	}

	int PKCS15DODF::searchOID(const ByteArray &oid, ByteArray &data) const
	{
		int result = -1;
		map<ByteArray, PKCS15OID>::const_iterator item;

		item = mapOID.find(oid);
		if (item != mapOID.end())
		{
			data = item->second.getPath();
			result = 0;
		}

		return result;
	}

} /* namespace smartcard_service_api */
