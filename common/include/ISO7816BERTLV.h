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

#ifndef ISO7816BERTLV_H_
#define ISO7816BERTLV_H_

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
	class LIBSCL_EXPORT_API ISO7816BERTLV : public TLVHelper
	{
	private:
		unsigned char firstByte;
		unsigned int tagClass;
		unsigned int encoding;

		ISO7816BERTLV(TLVHelper *parent);
		ISO7816BERTLV(const ByteArray &array, TLVHelper *parent);

		int decodeTag(const unsigned char *buffer);
		int decodeLength(const unsigned char *buffer);
		int decodeValue(const unsigned char *buffer);

		TLVHelper *getChildTLV(const ByteArray &data);

	public:
		ISO7816BERTLV();
		ISO7816BERTLV(const ByteArray &array);
		~ISO7816BERTLV();

		unsigned int getClass() const;
		unsigned int getEncoding() const;

		static const ByteArray encode(unsigned int tagClass, unsigned int encoding, unsigned int tag, const ByteArray &buffer);
		static const ByteArray encode(unsigned int tagClass, unsigned int encoding, unsigned int tag, unsigned char *buffer, unsigned int length);
	};

} /* namespace smartcard_service_api */
#endif /* ISO7816BERTLV_H_ */
