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

#ifndef SIMPLETLV_H_
#define SIMPLETLV_H_

/* standard library header */

/* SLP library header */

/* local header */
#include "ByteArray.h"
#include "TLVHelper.h"

#ifndef LIBSCL_EXPORT_API
#define LIBSCL_EXPORT_API
#endif // LIBSCL_EXPORT_API


namespace smartcard_service_api
{
	class LIBSCL_EXPORT_API SimpleTLV : public TLVHelper
	{
	private:
		SimpleTLV(TLVHelper *parent);
		SimpleTLV(const ByteArray &array, TLVHelper *parent);

		int decodeTag(const unsigned char *buffer);
		int decodeLength(const unsigned char *buffer);
		int decodeValue(const unsigned char *buffer);

		TLVHelper *getChildTLV(const ByteArray &data);

	public:
		SimpleTLV();
		SimpleTLV(const ByteArray &array);
		~SimpleTLV();

		static const ByteArray getOctetString(const ByteArray &array);
		static bool getBoolean(const ByteArray &array);
		static int getInteger(const ByteArray &array);

		static const ByteArray getOctetString(SimpleTLV &tlv);
		static bool getBoolean(SimpleTLV &tlv);
		static int getInteger(SimpleTLV &tlv);

		static const ByteArray encode(unsigned int tag, const ByteArray &buffer);
		static const ByteArray encode(unsigned int tag, unsigned char *buffer, unsigned int length);
	};

} /* namespace smartcard_service_api */
#endif /* SIMPLETLV_H_ */
