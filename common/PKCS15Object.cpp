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
#include "SimpleTLV.h"
#include "PKCS15Object.h"

namespace smartcard_service_api
{
//	PKCS15Object::PKCS15Object():FileObject()
//	{
//	}

	PKCS15Object::PKCS15Object(Channel *channel):FileObject(channel)
	{
	}

	PKCS15Object::PKCS15Object(Channel *channel, ByteArray selectResponse):FileObject(channel, selectResponse)
	{
	}

	PKCS15Object::~PKCS15Object()
	{
	}

	ByteArray PKCS15Object::getOctetStream(const ByteArray &data)
	{
		ByteArray result;
		SimpleTLV tlv(data);

		if (tlv.decodeTLV() && tlv.getTag() == TAG_SEQUENCE)
		{
			tlv.enterToValueTLV();

			if (tlv.decodeTLV() && tlv.getTag() == TAG_OCTET_STREAM)
			{
				result = tlv.getValue();
			}
			else
			{
				_ERR("TAG_OCTET_STREAM not found");
			}
			tlv.returnToParentTLV();
		}
		else
		{
			_ERR("TAG_SEQUENCE not found");
		}

		return result;
	}


} /* namespace smartcard_service_api */
