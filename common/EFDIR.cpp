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

#include <stdio.h>

#include "Debug.h"
#include "EFDIR.h"
#include "APDUHelper.h"
#include "SimpleTLV.h"

namespace smartcard_service_api
{
	static unsigned char path_efdir[] = { 0x2f, 0x00 };
	static ByteArray PATH_EFDIR(ARRAY_AND_SIZE(path_efdir));

	EFDIR::EFDIR(Channel *channel) : FileObject(channel)
	{
	}

	EFDIR::EFDIR(Channel *channel, const ByteArray &selectResponse) :
		FileObject(channel, selectResponse)
	{
	}

	EFDIR::~EFDIR()
	{
	}

	int EFDIR::select()
	{
		int ret;

		ret = FileObject::select(PATH_EFDIR, false);
		if (ret < SCARD_ERROR_OK)
		{
			_ERR("EFDIR select failed, [%d]", ret);
		}

		return ret;
	}

	const ByteArray EFDIR::parseRecord(const Record &record, const ByteArray &aid)
	{
		bool matched = false;
		ByteArray result;
		SimpleTLV tlv(record.getData());

		if (tlv.decodeTLV() == true && tlv.getTag() == 0x61)
		{
			tlv.enterToValueTLV();
			while (tlv.decodeTLV() == true)
			{
				switch (tlv.getTag())
				{
				case 0x4F : /* aid */
					if (tlv.getValue() == aid)
						matched = true;
					break;
				case 0x50 : /* label */
					break;
				case 0x51 : /* path */
					result = tlv.getValue();
					break;
				case 0x53 : /* ddo */
					break;
				}
			}
			tlv.returnToParentTLV();

			if (matched == true)
			{
				_DBG("Found!! : path %s", result.toString().c_str());
			}
			else
			{
				result.clear();
			}
		}

		return result;
	}

	const ByteArray EFDIR::getPathByAID(const ByteArray &aid)
	{
		ByteArray result;
		Record record;
		unsigned int id;
		int status = 0;

		for (id = 1; status >= 0; id++)
		{
			status = readRecord(0, id, record);
			if (status >= 0)
			{
				result = parseRecord(record, aid);
				if (result.size() > 0)
					break;
			}
		}

		return result;
	}
} /* namespace smartcard_service_api */
