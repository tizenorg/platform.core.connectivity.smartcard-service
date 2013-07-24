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
#include <string>
#include <stdint.h>
#include <stddef.h>

/* SLP library header */

/* local header */
//#include "Serializable.h"

#define ARRAY_AND_SIZE(x) (uint8_t *)(&x), sizeof(x)

using namespace std;

namespace smartcard_service_api
{
	class ByteArray //: public Serializable
	{
	protected:
		uint8_t *buffer;
		size_t length;

		bool _assign(uint8_t *array, size_t size);
		void save(const char *filePath);

	public:
		static ByteArray EMPTY;

		ByteArray();
		ByteArray(const uint8_t *array, size_t size);
		ByteArray(const ByteArray &T);
		~ByteArray();

//		ByteArray serialize();
//		void deserialize(ByteArray buffer);

		bool assign(const uint8_t *array, size_t size);
		inline bool setBuffer(const uint8_t *array, size_t size) { return assign(array, size); }
		void clear();

		size_t size() const;
		inline size_t getLength() const { return size(); }
		uint8_t *getBuffer();
		inline const uint8_t *getBuffer() const { return getBuffer(); }
		uint8_t *getBuffer(size_t offset);
		inline const uint8_t *getBuffer(size_t offset) const { return getBuffer(offset); }

		uint8_t at(size_t index) const;
		uint8_t reverseAt(size_t index) const;

		size_t extract(uint8_t *array, size_t size) const;

		const ByteArray sub(size_t offset, size_t size) const;

		/* operator overloading */
		ByteArray &operator =(const ByteArray &T);
		ByteArray operator +(const ByteArray &T);
		ByteArray &operator +=(const ByteArray &T);
		bool operator ==(const ByteArray &T) const;
		bool operator !=(const ByteArray &T) const;
		bool operator <(const ByteArray &T) const;
		bool operator >(const ByteArray &T) const;
		uint8_t operator [](size_t index) const;

		inline bool isEmpty() const { return (buffer == (void *)0 || length == 0); }
		const string toString() const;
	};

} /* namespace smartcard_service_api */
#endif /* BYTEARRAY_H_ */
