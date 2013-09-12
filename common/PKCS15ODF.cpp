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
#include "PKCS15ODF.h"
#include "SimpleTLV.h"
#include "NumberStream.h"

namespace smartcard_service_api
{
	PKCS15ODF::PKCS15ODF(Channel *channel) :
		PKCS15Object(channel), dodf(NULL)
	{
		int ret = 0;

		if ((ret = select(PKCS15ODF::ODF_FID)) >= SCARD_ERROR_OK)
		{
			ByteArray odfData, extra;

			_DBG("response : %s", selectResponse.toString().c_str());

			if ((ret = readBinary(0, 0, getFCP()->getFileSize(), odfData)) == 0)
			{
				_DBG("odfData : %s", odfData.toString().c_str());

				parseData(odfData);
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

	PKCS15ODF::PKCS15ODF(Channel *channel, const ByteArray &selectResponse) :
		PKCS15Object(channel, selectResponse), dodf(NULL)
	{
		int ret = 0;
		ByteArray odfData;

		if ((ret = readBinary(0, 0, 0, odfData)) == 0)
		{
			_DBG("odfData : %s", odfData.toString().c_str());

			parseData(odfData);
		}
		else
		{
			_ERR("readBinary failed, [%d]", ret);
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

	bool PKCS15ODF::parseData(const ByteArray &data)
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

					_DBG("TAG_DODF");

					dodf = PKCS15Object::getOctetStream(tlv.getValue());

					_DBG("path : %s", dodf.toString().c_str());

					pair<unsigned int, ByteArray> newPair(tlv.getTag(), dodf);
					dataList.insert(newPair);
				}
				break;

			case (unsigned int)0xA5 ://PKCS15ODF::TAG_TOKENINFO :
				{
					ByteArray tokeninfo;

					_DBG("TAG_TOKENINFO");

					tokeninfo = PKCS15Object::getOctetStream(tlv.getValue());

					_DBG("path : %s", tokeninfo.toString().c_str());

					pair<unsigned int, ByteArray> newPair(tlv.getTag(), tokeninfo);
					dataList.insert(newPair);
				}
				break;

			default :
				_DBG("Unknown tlv : t [%X], l [%d], v %s",
					tlv.getTag(), tlv.size(), tlv.getValue().toString().c_str());
				break;
			}

		}

		_INFO("dataList.size() = %d", dataList.size());

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

				_DBG("fid [%X]", fid);

				dodf = new PKCS15DODF(fid, channel);
				if (dodf != NULL && dodf->isClosed() == true)
				{
					_ERR("failed to open DODF");

					delete dodf;
					dodf = NULL;
				}
			}
			else
			{
				_ERR("[%02X] is not found. total [%d]", TAG_DODF, dataList.size());
			}
		}

		_DBG("dodf [%p]", dodf);

		return dodf;
	}

} /* namespace smartcard_service_api */
