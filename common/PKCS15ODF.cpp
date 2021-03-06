/*
* Copyright (c) 2012 Samsung Electronics Co., Ltd All Rights Reserved
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
#include "PKCS15ODF.h"
#include "SimpleTLV.h"
#include "NumberStream.h"

namespace smartcard_service_api
{
//	PKCS15ODF::PKCS15ODF():PKCS15Object()
//	{
//
//	}

	PKCS15ODF::PKCS15ODF(Channel *channel):PKCS15Object(channel), dodf(NULL)
	{
		int ret = 0;

		if ((ret = select(PKCS15ODF::ODF_FID)) == 0)
		{
			ByteArray odfData, extra;

			SCARD_DEBUG("response : %s", selectResponse.toString());

			if ((ret = readBinary(0, 0, getFCP()->getFileSize(), odfData)) == 0)
			{
				SCARD_DEBUG("odfData : %s", odfData.toString());

				parseData(odfData);
			}
			else
			{
				SCARD_DEBUG_ERR("readBinary failed, [%d]", ret);
			}
		}
		else
		{
			SCARD_DEBUG_ERR("select failed, [%d]", ret);
		}
	}

	PKCS15ODF::PKCS15ODF(Channel *channel, ByteArray selectResponse):PKCS15Object(channel, selectResponse), dodf(NULL)
	{
		int ret = 0;
		ByteArray odfData;

		if ((ret = readBinary(0, 0, 0, odfData)) == 0)
		{
			SCARD_DEBUG("odfData : %s", odfData.toString());

			parseData(odfData);
		}
		else
		{
			SCARD_DEBUG_ERR("readBinary failed, [%d]", ret);
		}
	}

	PKCS15ODF::~PKCS15ODF()
	{
		if (dodf != NULL)
		{
			delete dodf;
			dodf = NULL;
		}
	}

	bool PKCS15ODF::parseData(ByteArray data)
	{
		bool result = false;
		SimpleTLV tlv(data);

		while (tlv.decodeTLV())
		{
			switch (tlv.getTag())
			{
			case (unsigned int)0xA7 ://PKCS15ODF::TAG_DODF :
				{
					ByteArray dodf;

					SCARD_DEBUG("TAG_DODF");

					dodf = PKCS15Object::getOctetStream(tlv.getValue());

					SCARD_DEBUG("path : %s", dodf.toString());

					pair<unsigned int, ByteArray> newPair(tlv.getTag(), dodf);
					dataList.insert(newPair);
				}
				break;

			case (unsigned int)0xA5 ://PKCS15ODF::TAG_TOKENINFO :
				{
					ByteArray tokeninfo;

					SCARD_DEBUG("TAG_TOKENINFO");

					tokeninfo = PKCS15Object::getOctetStream(tlv.getValue());

					SCARD_DEBUG("path : %s", tokeninfo.toString());

					pair<unsigned int, ByteArray> newPair(tlv.getTag(), tokeninfo);
					dataList.insert(newPair);
				}
				break;

			default :
				SCARD_DEBUG("Unknown tlv : t [%X], l [%d], v %s", tlv.getTag(), tlv.getLength(), tlv.getValue().toString());
				break;
			}

		}

		SCARD_DEBUG("dataList.size() = %d", dataList.size());

		return result;
	}

	PKCS15DODF *PKCS15ODF::getDODF()
	{
		map<unsigned int, ByteArray>::iterator item;

		if (dodf == NULL)
		{
			item = dataList.find((unsigned int)0xA7/*PKCS15ODF::TAG_DODF*/);
			if (item != dataList.end())
			{
				NumberStream num(item->second);
				unsigned int fid = num.getLittleEndianNumber();

				SCARD_DEBUG("fid [%X]", fid);

				dodf = new PKCS15DODF(fid, channel);
			}
			else
			{
				SCARD_DEBUG_ERR("[%02X] is not found. total [%d]", TAG_DODF, dataList.size());
			}
		}

		SCARD_DEBUG("dodf [%p]", dodf);

		return dodf;
	}

} /* namespace smartcard_service_api */
