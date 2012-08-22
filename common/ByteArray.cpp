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

/* standard library header */
#include <stdio.h>
#include <string.h>
#include <errno.h>

/* SLP library header */

/* local header */
#include "Debug.h"
#include "ByteArray.h"

namespace smartcard_service_api
{
	ByteArray ByteArray::EMPTY = ByteArray();

	ByteArray::ByteArray()
	{
		buffer = NULL;
		length = 0;
	}

	ByteArray::ByteArray(uint8_t *array, uint32_t bufferLen)
	{
		buffer = NULL;
		length = 0;

		setBuffer(array, bufferLen);
	}

	ByteArray::ByteArray(const ByteArray &T)
	{
		buffer = NULL;
		length = 0;

		setBuffer(T.buffer, T.length);
	}

	ByteArray::~ByteArray()
	{
		releaseBuffer();
	}

	bool ByteArray::setBuffer(uint8_t *array, uint32_t bufferLen)
	{
		if (array == NULL || bufferLen == 0)
		{
			return false;
		}

		releaseBuffer();

		buffer = new uint8_t[bufferLen];
		if (buffer == NULL)
		{
			SCARD_DEBUG_ERR("alloc failed");
			return false;
		}

		memcpy(buffer, array, bufferLen);
		length = bufferLen;

		return true;
	}

	bool ByteArray::_setBuffer(uint8_t *array, uint32_t bufferLen)
	{
		if (array == NULL || bufferLen == 0)
		{
			return false;
		}

		releaseBuffer();

		buffer = array;
		length = bufferLen;

		return true;
	}

	uint32_t ByteArray::getLength() const
	{
		return length;
	}

	uint8_t *ByteArray::getBuffer() const
	{
		return getBuffer(0);
	}

	uint8_t *ByteArray::getBuffer(uint32_t offset) const
	{
		if (length == 0)
			return NULL;

		if (offset >= length)
		{
			SCARD_DEBUG_ERR("buffer overflow, offset [%d], length [%d]", offset, length);
			return NULL;
		}

		return buffer + offset;
	}

	uint8_t ByteArray::getAt(uint32_t index) const
	{
		if (index >= length)
		{
			SCARD_DEBUG_ERR("buffer overflow, index [%d], length [%d]", index, length);
			return buffer[length -1];
		}

		return buffer[index];
	}

	uint8_t ByteArray::getReverseAt(uint32_t index) const
	{
		if (index >= length)
		{
			SCARD_DEBUG_ERR("buffer underflow, index [%d], length [%d]", index, length);
			return buffer[0];
		}

		return buffer[length - index - 1];
	}

	uint32_t ByteArray::copyFromArray(uint8_t *array, uint32_t bufferLen) const
	{
		uint32_t min_len = 0;

		if (array == NULL || bufferLen == 0)
		{
			SCARD_DEBUG_ERR("invaild param");
			return false;
		}

		min_len = (bufferLen < length) ? bufferLen : length;

		memcpy(array, buffer, min_len);

		return min_len;
	}

	void ByteArray::releaseBuffer()
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
		uint32_t newLen;
		uint8_t *newBuffer;
		ByteArray newArray;

		if (length == 0)
		{
			SCARD_DEBUG("length is zero");

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
			SCARD_DEBUG_ERR("alloc failed");

			return *this;
		}

		memcpy(newBuffer, buffer, length);
		memcpy(newBuffer + length, T.buffer, T.length);

		newArray._setBuffer(newBuffer, newLen);

		return newArray;
	}

	ByteArray &ByteArray::operator =(const ByteArray &T)
	{
		if (this != &T)
		{
			setBuffer(T.buffer, T.length);
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

	uint8_t &ByteArray::operator [](uint32_t index) const
	{
		if (index >= length)
		{
			SCARD_DEBUG_ERR("buffer overflow, index [%d], length [%d]", index, length);
			return buffer[length -1];
		}

		return buffer[index];
	}

	const char *ByteArray::toString()
	{
		memset(strBuffer, 0, sizeof(strBuffer));

		if (length == 0)
		{
			snprintf(strBuffer, sizeof(strBuffer), "buffer is empty");
		}
		else
		{
			int count;
			int i, offset = 0;
			bool ellipsis = false;

			count = length;
			if (count > 20)
			{
				count = 20;
				ellipsis = true;
			}

			snprintf(strBuffer + offset, sizeof(strBuffer) - offset, "{ ");
			offset += 2;

			for (i = 0; i < count; i++)
			{
				snprintf(strBuffer + offset, sizeof(strBuffer) - offset, "%02X ", buffer[i]);
				offset += 3;
			}

			if (ellipsis)
			{
				snprintf(strBuffer + offset, sizeof(strBuffer) - offset, "... }");
			}
			else
			{
				snprintf(strBuffer + offset, sizeof(strBuffer) - offset, "}");
			}
		}

		return (const char *)strBuffer;
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
			SCARD_DEBUG("file has written, file [%s], length[%d]", filePath, length);
		}
		else
		{
			SCARD_DEBUG_ERR("file open failed, [%d]", errno);
		}
	}

} /* namespace smartcard_service_api */

