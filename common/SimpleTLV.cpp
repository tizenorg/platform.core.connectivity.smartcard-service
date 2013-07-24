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
#include <stdio.h>
#include <string.h>

/* SLP library header */

/* local header */
#include "Debug.h"
#include "SimpleTLV.h"

namespace smartcard_service_api
{
	SimpleTLV::SimpleTLV():TLVHelper()
	{
	}

	SimpleTLV::SimpleTLV(TLVHelper *parent):TLVHelper(parent)
	{
		parentTLV = parent;
	}

	SimpleTLV::SimpleTLV(const ByteArray &array):TLVHelper(array)
	{
	}

	SimpleTLV::SimpleTLV(const ByteArray &array, TLVHelper *parent):TLVHelper(array, parent)
	{
		parentTLV = parent;
	}

	SimpleTLV::~SimpleTLV()
	{
		if (childTLV != NULL)
		{
			delete childTLV;
			childTLV = NULL;
		}
	}

	int SimpleTLV::decodeTag(unsigned char *buffer)
	{
		/* 0x00 or 0xFF is invalid tag value */
		if (buffer[0] == 0x00 || buffer[0] == 0xFF)
		{
			return -1;
		}

		currentT = buffer[0];

		return 1;
	}

	int SimpleTLV::decodeLength(unsigned char *buffer)
	{
		int count = 0;

		if (buffer[0] == 0xFF)
		{
			/* 3 bytes length */
			currentL = (buffer[1] << 8) | buffer[2];
			count = 3;
		}
		else
		{
			/* 1 byte length */
			currentL = buffer[0];
			count = 1;
		}

		return count;
	}

	int SimpleTLV::decodeValue(unsigned char *buffer)
	{
		if (currentL == 0)
			return 0;

		currentV.setBuffer(buffer, currentL);

		return currentL;
	}

	ByteArray SimpleTLV::encode(unsigned int tag, ByteArray buffer)
	{
		bool isLongBuffer = false;
		ByteArray result;
		unsigned int total_len = 0;
		unsigned int current = 0;
		unsigned char *temp_buffer = NULL;

		/* add tag's length */
		total_len += 1;

		/* add length's length */
		if (buffer.getLength() < 255)
		{
			total_len += 1;
		}
		else if (buffer.getLength() < 65536)
		{
			total_len += 3;
			isLongBuffer = true;
		}
		else
		{
			return result;
		}

		/* add buffer's length */
		total_len += buffer.getLength();

		/* alloc new buffer */
		temp_buffer = new unsigned char[total_len];
		if (temp_buffer == NULL)
		{
			return result;
		}
		memset(temp_buffer, 0, total_len);

		/* fill tag */
		temp_buffer[current++] = (unsigned char)tag;

		/* fill length */
		if (isLongBuffer == true)
		{
			temp_buffer[current++] = (unsigned char)(0xFF);
			temp_buffer[current++] = (unsigned char)(buffer.getLength() >> 8);
			temp_buffer[current++] = (unsigned char)(buffer.getLength());
		}
		else
		{
			temp_buffer[current++] = (unsigned char)(buffer.getLength());
		}

		/* fill value */
		if (buffer.getLength() > 0)
			memcpy(temp_buffer + current, buffer.getBuffer(), buffer.getLength());

		result.setBuffer(temp_buffer, total_len);

		delete []temp_buffer;

		return result;
	}

	ByteArray SimpleTLV::encode(unsigned int tag, unsigned char *buffer, unsigned int length)
	{
		return encode(tag, ByteArray(buffer, length));
	}

	TLVHelper *SimpleTLV::getChildTLV(ByteArray data)
	{
		if (childTLV != NULL)
		{
			delete childTLV;
			childTLV = NULL;
		}
		childTLV = new SimpleTLV(data, this);

		return (TLVHelper *)childTLV;
	}

	ByteArray SimpleTLV::getOctetString(const ByteArray &array)
	{
		SimpleTLV tlv(array);

		return SimpleTLV::getOctetString(tlv);
	}

	ByteArray SimpleTLV::getOctetString(SimpleTLV &tlv)
	{
		ByteArray result;

		if (tlv.decodeTLV() == true && tlv.getTag() == 0x04) /* OCTET STRING */
		{
			result = tlv.getValue();
		}
		else
		{
			_ERR("getOctetString failed (0x%02X)", tlv.getTag());
		}

		return result;
	}

	bool SimpleTLV::getBoolean(const ByteArray &array)
	{
		SimpleTLV tlv(array);

		return SimpleTLV::getBoolean(tlv);
	}

	bool SimpleTLV::getBoolean(SimpleTLV &tlv)
	{
		bool result = false;

		if (tlv.decodeTLV() == true && tlv.getTag() == 0x80) /* BOOLEAN */
		{
			if (tlv.getValue().getAt(0) == 0)
				result = false;
			else
				result = true;
		}
		else
		{
			_ERR("getBoolean failed (0x%02X)", tlv.getTag());
		}

		return result;
	}

	int SimpleTLV::getInteger(const ByteArray &array)
	{
		SimpleTLV tlv(array);

		return SimpleTLV::getInteger(tlv);
	}

	int SimpleTLV::getInteger(SimpleTLV &tlv)
	{
		int result = 0;

		if (tlv.decodeTLV() == true && tlv.getTag() == 0x80) /* TODO : INTEGER */
		{
		}
		else
		{
			_ERR("getInteger failed (0x%02X)", tlv.getTag());
		}

		return result;
	}
} /* namespace smartcard_service_api */
