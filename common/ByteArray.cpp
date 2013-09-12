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
#include <cstdio>
#include <cstring>
#include <cerrno>
#include <sstream>

/* SLP library header */

/* local header */
#include "Debug.h"
#include "ByteArray.h"

namespace smartcard_service_api
{
	ByteArray ByteArray::EMPTY = ByteArray();

	ByteArray::ByteArray() : buffer(NULL), length(0)
	{
	}

	ByteArray::ByteArray(const uint8_t *array, size_t size) :
		buffer(NULL), length(0)
	{
		assign(array, size);
	}

	ByteArray::ByteArray(const ByteArray &T) : buffer(NULL), length(0)
	{
		assign(T.buffer, T.length);
	}

	ByteArray::~ByteArray()
	{
		clear();
	}

	bool ByteArray::assign(const uint8_t *array, size_t size)
	{
		if (array == NULL || size == 0)
		{
			return false;
		}

		clear();

		buffer = new uint8_t[size];
		if (buffer == NULL)
		{
			_ERR("alloc failed");
			return false;
		}

		memcpy(buffer, array, size);
		length = size;

		return true;
	}

	bool ByteArray::_assign(uint8_t *array, size_t size)
	{
		if (array == NULL || size == 0)
		{
			return false;
		}

		clear();

		buffer = array;
		length = size;

		return true;
	}

	size_t ByteArray::size() const
	{
		return length;
	}

	uint8_t *ByteArray::getBuffer()
	{
		return getBuffer(0);
	}

	const uint8_t *ByteArray::getBuffer() const
	{
		return getBuffer(0);
	}

	uint8_t *ByteArray::getBuffer(size_t offset)
	{
		if (length == 0)
			return NULL;

		if (offset >= length)
		{
			_ERR("buffer overflow, offset [%d], length [%d]", offset, length);
			return NULL;
		}

		return buffer + offset;
	}

	const uint8_t *ByteArray::getBuffer(size_t offset) const
	{
		if (length == 0)
			return NULL;

		if (offset >= length)
		{
			_ERR("buffer overflow, offset [%d], length [%d]", offset, length);
			return NULL;
		}

		return buffer + offset;
	}

	uint8_t ByteArray::at(size_t index) const
	{
		if (index >= length)
		{
			_ERR("buffer overflow, index [%d], length [%d]", index, length);
			if (length > 0)
			{
				return buffer[length - 1];
			}
			else
			{
				return 0;
			}
		}

		return buffer[index];
	}

	uint8_t ByteArray::reverseAt(size_t index) const
	{
		if (index >= length)
		{
			_ERR("buffer underflow, index [%d], length [%d]", index, length);
			if (length > 0)
			{
				return buffer[0];
			}
			else
			{
				return 0;
			}
		}

		return buffer[length - index - 1];
	}

	size_t ByteArray::extract(uint8_t *array, size_t size) const
	{
		uint32_t min_len = 0;

		if (array == NULL || size == 0)
		{
			_ERR("invalid param");
			return min_len;
		}

		min_len = (size < length) ? size : length;

		memcpy(array, buffer, min_len);

		return min_len;
	}

	const ByteArray ByteArray::sub(size_t offset, size_t size) const
	{
		if (length == 0 || offset >= length || (offset + size) > length)
		{
			_DBG("length is zero");

			return ByteArray();
		}

		return ByteArray(buffer + offset, size);
	}

	void ByteArray::clear()
	{
		if (buffer != NULL)
		{
			delete []buffer;
			buffer = NULL;
		}
		length = 0;
	}

	/* operator overloading */
	ByteArray ByteArray::operator +(const ByteArray &T)
	{
		size_t newLen;
		uint8_t *newBuffer;
		ByteArray newArray;

		if (length == 0)
		{
			_DBG("length is zero");

			return T;
		}

		newLen = T.length;

		if (newLen == 0)
			return *this;

		newLen += length;

		newBuffer = new uint8_t[newLen];
		if (newBuffer == NULL)
		{
			/* assert.... */
			_ERR("alloc failed");

			return *this;
		}

		memcpy(newBuffer, buffer, length);
		memcpy(newBuffer + length, T.buffer, T.length);

		newArray._assign(newBuffer, newLen);

		return newArray;
	}

	ByteArray &ByteArray::operator =(const ByteArray &T)
	{
		if (this != &T)
		{
			assign(T.buffer, T.length);
		}

		return *this;
	}

	ByteArray &ByteArray::operator +=(const ByteArray &T)
	{
		*this = *this + T;

		return *this;
	}

	bool ByteArray::operator ==(const ByteArray &T) const
	{
		if (length != T.length)
			return false;

		return (memcmp(buffer, T.buffer, length) == 0);
	}

	bool ByteArray::operator !=(const ByteArray &T) const
	{
		return !(*this == T);
	}

	bool ByteArray::operator <(const ByteArray &T) const
	{
		return (memcmp(buffer, T.buffer, (length < T.length) ? length : T.length) < 0);
	}

	bool ByteArray::operator >(const ByteArray &T) const
	{
		return (memcmp(buffer, T.buffer, (length < T.length) ? length : T.length) > 0);
	}

	uint8_t ByteArray::operator [](size_t index) const
	{
		if (index >= length)
		{
			_ERR("buffer overflow, index [%d], length [%d]", index, length);
			if (length > 0)
			{
				return buffer[length -1];
			}
			else
			{
				return 0;
			}
		}

		return buffer[index];
	}
	const string ByteArray::toString() const
	{
		stringstream ss;

		if (length > 0)
		{
			char temp[20];
			int count;
			int i = 0;
			bool ellipsis = false;

			count = length;
			if (count > 20)
			{
				count = 20;
				ellipsis = true;
			}

			ss << "{ ";

			for (i = 0; i < count; i++)
			{
				snprintf(temp, sizeof(temp), "%02X ", buffer[i]);
				ss << temp;
			}

			if (ellipsis)
			{
				ss << "... ";
			}

			ss << "}";
		}
		else
		{
			ss << "buffer is empty";
		}

		return ss.str();
	}

	void ByteArray::save(const char *filePath)
	{
		FILE *file = NULL;

		if (filePath == NULL || buffer == NULL || length == 0)
			return;

		if ((file = fopen(filePath, "w")) != NULL)
		{
			fwrite(buffer, 1, length, file);
			fflush(file);
			fclose(file);
		}
		else
		{
			_ERR("file open failed, [%d]", errno);
		}
	}

} /* namespace smartcard_service_api */

