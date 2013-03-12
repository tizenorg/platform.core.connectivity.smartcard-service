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
#include "PKCS15Path.h"

namespace smartcard_service_api
{
	PKCS15Path::PKCS15Path()
	{
	}

	PKCS15Path::~PKCS15Path()
	{
	}

//	PKCS15Path(ByteArray &data);
//	PKCS15Path(ByteArray path, int index);
//	PKCS15Path(unsigned char *path, unsigned int length, int index);
//	~PKCS15Path();
//
//	bool PKCS15Path::parseData(ByteArray &data)
//	{
//		SimpleTLV tlv(data);
//
//		if (tlv.decodeTLV() == true && tlv.getTag() == 0x30) /* SEQUENCE */
//		{
//			/* get path */
//			path = tlv.getOctetString();
//
//			if (tlv.decodeTLV())
//			index = t
//
//		}
//	}
//
//	int getPath(ByteArray &path);
//	bool hasIndexLength();
//	int getIndex();
//	unsigned int getLength();
//	int encode(ByteArray &result);

} /* namespace smartcard_service_api */
