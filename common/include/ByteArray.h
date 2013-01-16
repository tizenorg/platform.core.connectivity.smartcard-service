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


#ifndef BYTEARRAY_H_
#define BYTEARRAY_H_

/* standard library header */
#include <stdint.h>

/* SLP library header */

/* local header */
//#include "Serializable.h"

#define ARRAY_AND_SIZE(x) (uint8_t *)(&x), sizeof(x)

namespace smartcard_service_api
{
	class ByteArray //: public Serializable
	{
	protected:
		uint8_t *buffer;
		uint32_t length;
		char strBuffer[100];

		bool _setBuffer(uint8_t *array, uint32_t bufferLen);
		void save(const char *filePath);

	public:
		static ByteArray EMPTY;

		ByteArray();
		ByteArray(uint8_t *array, uint32_t bufferLen);
		ByteArray(const ByteArray &T);
		~ByteArray();

//		ByteArray serialize();
//		void deserialize(ByteArray buffer);

		bool setBuffer(uint8_t *array, uint32_t bufferLen);
		void releaseBuffer();

		uint32_t getLength() const;
		uint8_t *getBuffer() const;
		uint8_t *getBuffer(uint32_t offset) const;

		uint8_t getAt(uint32_t index) const;
		uint8_t getReverseAt(uint32_t index) const;

		uint32_t copyFromArray(uint8_t *array, uint32_t bufferLen) const;

		/* operator overloading */
		ByteArray &operator =(const ByteArray &T);
		ByteArray operator +(const ByteArray &T);
		ByteArray &operator +=(const ByteArray &T);
		bool operator ==(const ByteArray &T) const;
		bool operator !=(const ByteArray &T) const;
		bool operator <(const ByteArray &T) const;
		bool operator >(const ByteArray &T) const;
		uint8_t &operator [](uint32_t index) const;

		inline bool isEmpty() { return (buffer == (void *)0 || length == 0); }
		const char *toString();
	};

} /* namespace smartcard_service_api */
#endif /* BYTEARRAY_H_ */
