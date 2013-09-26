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

#include "PKCS15Path.h"

namespace smartcard_service_api
{
	PKCS15Path::PKCS15Path() : index(-1), length(0)
	{
	}

	PKCS15Path::PKCS15Path(const ByteArray &data) : index(-1), length(0)
	{
		parseData(data);
	}

	PKCS15Path::PKCS15Path(const ByteArray &path, int index) :
		path(path), index(index), length(0)
	{
	}

	PKCS15Path::PKCS15Path(const unsigned char *path,
		size_t length, int index) :
		path(path, length), index(index), length(0)
	{
	}

	PKCS15Path::~PKCS15Path()
	{
	}

	bool PKCS15Path::parseData(const ByteArray &data)
	{
		/* TODO */
//		SimpleTLV tlv(data);
//
//		if (tlv.decodeTLV() == true && tlv.getTag() == 0x30) /* SEQUENCE */
//		{
//			/* get path */
//			path = tlv.getOctetString();
//
//			if (tlv.decodeTLV())
//		}
		return true;
	}

	int PKCS15Path::encode(ByteArray &result) const
	{
		/* TODO */
		return 0;
	}

} /* namespace smartcard_service_api */
