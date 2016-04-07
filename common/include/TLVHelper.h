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

#ifndef TLVHELPER_H_
#define TLVHELPER_H_

/* standard library header */
#include <stddef.h>

/* SLP library header */

/* local header */
#include "ByteArray.h"

#ifndef LIBSCL_EXPORT_API
#define LIBSCL_EXPORT_API
#endif // LIBSCL_EXPORT_API


namespace smartcard_service_api
{
	class LIBSCL_EXPORT_API TLVHelper
	{
	protected:
		TLVHelper *currentTLV;
		TLVHelper *parentTLV;
		TLVHelper *childTLV;

		ByteArray tlvBuffer;
		unsigned int offset;

		unsigned int currentT;
		unsigned int currentL;
		ByteArray currentV;

		TLVHelper(TLVHelper *parent);
		TLVHelper(const ByteArray &array, TLVHelper *parent);

		virtual int decodeTag(const unsigned char *buffer) = 0;
		virtual int decodeLength(const unsigned char *buffer) = 0;
		virtual int decodeValue(const unsigned char *buffer) = 0;

		virtual TLVHelper *getChildTLV(const ByteArray &data) = 0;
		TLVHelper *getParentTLV();

		bool setTLVBuffer(const ByteArray &array, TLVHelper *parent);
		bool setTLVBuffer(const unsigned char *buffer, unsigned int length, TLVHelper *parent);

		inline bool _isEndOfBuffer() const { return offset >= tlvBuffer.size(); }
		bool _decodeTLV();

		inline unsigned int _getTag() const { return currentT; }
		inline unsigned int _size() const { return currentL; }
		inline const ByteArray _getValue() const { return currentV; }

	public:
		TLVHelper();
		TLVHelper(const ByteArray &array);
		virtual ~TLVHelper();

		bool setTLVBuffer(const ByteArray &array) { return setTLVBuffer(array, NULL); }
		bool setTLVBuffer(const unsigned char *buffer, unsigned int length) { return setTLVBuffer(buffer, length, NULL); }

		inline bool isEndOfBuffer() const { return currentTLV->_isEndOfBuffer(); }
		inline bool decodeTLV() { return currentTLV->_decodeTLV(); }

		unsigned int getTag() const { return currentTLV->_getTag(); }
		unsigned int size() const { return currentTLV->_size(); }
		const ByteArray getValue() const { return currentTLV->_getValue(); }

		const string toString() const;

		bool enterToValueTLV();
		bool returnToParentTLV();
	};

} /* namespace smartcard_service_api */
#endif /* TLVHELPER_H_ */
