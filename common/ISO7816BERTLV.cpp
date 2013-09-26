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

#include <stdio.h>
#include <string.h>

#include "ISO7816BERTLV.h"

namespace smartcard_service_api
{
	ISO7816BERTLV::ISO7816BERTLV() : TLVHelper(), firstByte(0), tagClass(0),
		encoding(0)
	{
	}

	ISO7816BERTLV::ISO7816BERTLV(TLVHelper *parent) : TLVHelper(parent),
		firstByte(0), tagClass(0), encoding(0)
	{
	}

	ISO7816BERTLV::ISO7816BERTLV(const ByteArray &array) : TLVHelper(array),
			firstByte(0), tagClass(0), encoding(0)
	{
	}

	ISO7816BERTLV::ISO7816BERTLV(const ByteArray &array, TLVHelper *parent) :
		TLVHelper(array, parent), firstByte(0), tagClass(0), encoding(0)
	{
	}

	ISO7816BERTLV::~ISO7816BERTLV()
	{
		if (childTLV != NULL)
		{
			delete childTLV;
			childTLV = NULL;
		}
	}

	int ISO7816BERTLV::decodeTag(const unsigned char *buffer)
	{
		/* 0x00 is invalid tag value */
		if (buffer[0] == 0x00)
		{
			return -1;
		}

		/* first byte */
		tagClass = (buffer[0] & 0xC0) >> 6;
		encoding = (buffer[0] & 0x20) >> 5;

		currentT = buffer[0];

		if ((buffer[0] & 0x1F) < 0x1F)
		{
			return 1;
		}

		/* second byte */
		currentT = (currentT << 8) | buffer[1];
		if (buffer[1] & 0x80)
		{
			/* third byte */
			if (buffer[2] & 0x80)
			{
				return -1;
			}

			currentT = (currentT << 8) | buffer[2];

			return 3;
		}

		return 2;
	}

	int ISO7816BERTLV::decodeLength(const unsigned char *buffer)
	{
		if (buffer[0] & 0x80)
		{
			uint8_t count = (buffer[0] & 0x7F);
			uint8_t i;

			/* count will be less than 5 */
			if (count > 4)
				return -1;

			count++;

			for (i = 1; i < count; i++)
			{
				/* if big endian */
				currentL = (currentL << 8) | buffer[i];

				/* if little endian */
				/* currentL = currentL | (buffer[i] << (8 * (i - 1))); */
			}

			return count;
		}
		else
		{
			currentL = buffer[0];

			return 1;
		}
	}

	int ISO7816BERTLV::decodeValue(const unsigned char *buffer)
	{
		if (currentL == 0)
			return 0;

		currentV.assign(buffer, currentL);

		return currentL;
	}

	unsigned int ISO7816BERTLV::getClass() const
	{
		return tagClass;
	}

	unsigned int ISO7816BERTLV::getEncoding() const
	{
		return encoding;
	}

	const ByteArray ISO7816BERTLV::encode(unsigned int tagClass, unsigned int encoding, unsigned int tag, const ByteArray &buffer)
	{
		unsigned char temp_tag[3] = { 0, };
		unsigned char temp_tag_len = 0;
		unsigned char temp_len[5] = { 0, };
		unsigned char temp_len_len = 0;
		ByteArray result;
		unsigned int total_len = 0;
		unsigned int current = 0;
		unsigned char *temp_buffer = NULL;

		/* add tag's length */
		if (tag > 0x7FFF)
			return result;

		temp_tag[0] = (tagClass << 6) | (encoding << 5);

		if (tag < 0x1F)
		{
			temp_tag[0] |= tag;
			temp_tag_len = 1;
		}
		else
		{
			temp_tag[0] |= 0x1F;

			if (tag < 0x80)
			{
				temp_tag[1] = tag;

				temp_tag_len = 2;
			}
			else
			{
				temp_tag[1] = (tag & 0x000000FF);
				temp_tag[2] = (tag & 0x0000FF00);

				temp_tag_len = 3;
			}
		}

		total_len += temp_tag_len;

		/* add length's length */
		if (buffer.size() < 128)
		{
			temp_len[0] = buffer.size();

			temp_len_len = 1;
		}
		else
		{
			temp_len[0] = 0x80;
			temp_len_len = 1;

			if (buffer.size() > 0x00FFFFFF)
			{
				temp_len[4] = (buffer.size() & 0xFF000000) >> 24;
				temp_len_len++;
			}

			if (buffer.size() > 0x0000FFFF)
			{
				temp_len[3] = (buffer.size() & 0x00FF0000) >> 16;
				temp_len_len++;
			}

			if (buffer.size() > 0x000000FF)
			{
				temp_len[2] = (buffer.size() & 0x0000FF00) >> 8;
				temp_len_len++;
			}

			temp_len[1] = buffer.size() & 0x000000FF;
			temp_len_len++;

			temp_len[0] |= temp_len_len;
		}

		/* add buffer's length */
		total_len += buffer.size();

		/* alloc new buffer */
		temp_buffer = new unsigned char[total_len];
		if (temp_buffer == NULL)
		{
			return result;
		}
		memset(temp_buffer, 0, total_len);

		/* fill tag */
		memcpy(temp_buffer + current, temp_tag, temp_tag_len);
		current += temp_tag_len;

		/* fill length */
		memcpy(temp_buffer + current, temp_len, temp_len_len);
		current += temp_len_len;

		/* fill value */
		if (buffer.size() > 0)
			memcpy(temp_buffer + current, buffer.getBuffer(), buffer.size());

		result.assign(temp_buffer, total_len);

		delete []temp_buffer;

		return result;
	}

	const ByteArray ISO7816BERTLV::encode(unsigned int tagClass, unsigned int encoding, unsigned int tag, unsigned char *buffer, unsigned int length)
	{
		return encode(tagClass, encoding, tag, ByteArray(buffer, length));
	}

	TLVHelper *ISO7816BERTLV::getChildTLV(const ByteArray &data)
	{
		if (childTLV != NULL)
		{
			delete childTLV;
			childTLV = NULL;
		}
		childTLV = new ISO7816BERTLV(data, this);

		return (TLVHelper *)childTLV;
	}

} /* namespace smartcard_service_api */
