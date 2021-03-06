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


#ifndef ISO7816BERTLV_H_
#define ISO7816BERTLV_H_

/* standard library header */

/* SLP library header */

/* local header */
#include "ByteArray.h"
#include "TLVHelper.h"

namespace smartcard_service_api
{
	class ISO7816BERTLV: public TLVHelper
	{
	private:
//		ISO7816BERTLV child;

		unsigned char firstByte;
		unsigned int tagClass;
		unsigned int encoding;

		ISO7816BERTLV(TLVHelper *parent);
		ISO7816BERTLV(const ByteArray &array, TLVHelper *parent);

		int decodeTag(unsigned char *buffer);
		int decodeLength(unsigned char *buffer);
		int decodeValue(unsigned char *buffer);

		TLVHelper *getChildTLV(ByteArray data);

	public:
		ISO7816BERTLV();
		ISO7816BERTLV(const ByteArray &array);
		~ISO7816BERTLV();

		unsigned int getClass();
		unsigned int getEncoding();

		static ByteArray encode(unsigned int tagClass, unsigned int encoding, unsigned int tag, ByteArray buffer);
		static ByteArray encode(unsigned int tagClass, unsigned int encoding, unsigned int tag, unsigned char *buffer, unsigned int length);
	};

} /* namespace smartcard_service_api */
#endif /* ISO7816BERTLV_H_ */
