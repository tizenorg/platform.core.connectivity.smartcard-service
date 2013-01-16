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

namespace smartcard_service_api
{
	class TLVHelper
	{
	protected:
		TLVHelper *currentTLV;
		TLVHelper *parentTLV;
		TLVHelper *childTLV;

		char strBuffer[200];
		ByteArray tlvBuffer;
		unsigned int offset;

		unsigned int currentT;
		unsigned int currentL;
		ByteArray currentV;

		void initialize(TLVHelper *parent = NULL);
		TLVHelper(TLVHelper *parent);
		TLVHelper(const ByteArray &array, TLVHelper *parent);

		virtual int decodeTag(unsigned char *buffer) = 0;
		virtual int decodeLength(unsigned char *buffer) = 0;
		virtual int decodeValue(unsigned char *buffer) = 0;

		virtual TLVHelper *getChildTLV(ByteArray data) = 0;
		TLVHelper *getParentTLV();

		bool setTLVBuffer(const ByteArray &array, TLVHelper *parent);
		bool setTLVBuffer(unsigned char *buffer, unsigned int length, TLVHelper *parent);

		bool _isEndOfBuffer() { return offset >= tlvBuffer.getLength(); }
		bool _decodeTLV();

		unsigned int _getTag() { return currentT; }
		unsigned int _getLength() { return currentL; }
		ByteArray _getValue() { return currentV; }

	public:
		TLVHelper();
		TLVHelper(const ByteArray &array);
		~TLVHelper();

		bool setTLVBuffer(const ByteArray &array) { return setTLVBuffer(array, NULL); }
		bool setTLVBuffer(unsigned char *buffer, unsigned int length) { return setTLVBuffer(buffer, length, NULL); }

		bool isEndOfBuffer() { return currentTLV->_isEndOfBuffer(); }
		bool decodeTLV() { return currentTLV->_decodeTLV(); }

		unsigned int getTag() { return currentTLV->_getTag(); }
		unsigned int getLength() { return currentTLV->_getLength(); }
		ByteArray getValue() { return currentTLV->_getValue(); }

		const char *toString();

		bool enterToValueTLV();
		bool returnToParentTLV();
	};

} /* namespace smartcard_service_api */
#endif /* TLVHELPER_H_ */
