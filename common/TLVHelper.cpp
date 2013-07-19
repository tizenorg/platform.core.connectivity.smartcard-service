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
#include <sstream>

/* SLP library header */

/* local header */
#include "Debug.h"
#include "TLVHelper.h"

namespace smartcard_service_api
{
	TLVHelper::TLVHelper() : currentTLV(this), parentTLV(NULL),
		childTLV(NULL), offset(0), currentT(0), currentL(0)
	{
	}

	TLVHelper::TLVHelper(TLVHelper *parent) : currentTLV(this),
		parentTLV(parent), childTLV(NULL),
		offset(0), currentT(0), currentL(0)
	{
	}

	TLVHelper::TLVHelper(const ByteArray &array) : currentTLV(this),
		parentTLV(NULL), childTLV(NULL),
		offset(0), currentT(0), currentL(0)
	{
		setTLVBuffer(array);
	}

	TLVHelper::TLVHelper(const ByteArray &array, TLVHelper *parent) :
		currentTLV(this), parentTLV(NULL), childTLV(NULL),
		offset(0), currentT(0), currentL(0)
	{
		setTLVBuffer(array, parent);
	}

	TLVHelper::~TLVHelper()
	{
	}

	bool TLVHelper::setTLVBuffer(const ByteArray &array, TLVHelper *parent)
	{
		if (array.size() == 0)
			return false;

		currentTLV = this;
		parentTLV = parent;
		childTLV = NULL;
		offset = 0;
		currentT = 0;
		currentL = 0;

		tlvBuffer = array;

		return true;
	}

	bool TLVHelper::setTLVBuffer(const unsigned char *buffer, unsigned int length, TLVHelper *parent)
	{
		return setTLVBuffer(ByteArray(buffer, length), parent);
	}

	bool TLVHelper::_decodeTLV()
	{
		int result;
		int temp = 0;

		currentT = 0;
		currentL = 0;
		currentV.clear();

		if (isEndOfBuffer())
			return false;

		/* T */
		if ((result = decodeTag(tlvBuffer.getBuffer(offset + temp))) < 0)
			return false;

		temp += result;

		/* L */
		if ((result = decodeLength(tlvBuffer.getBuffer(offset + temp))) < 0)
			return false;

		temp += result;

		if (currentL > 0)
		{
			if (currentL > (tlvBuffer.size() - (offset + temp)))
				return false;

			/* V */
			if ((result = decodeValue(tlvBuffer.getBuffer(offset + temp))) < 0)
				return false;

			temp += result;
		}

		offset += temp;

		return true;
	}

	const string TLVHelper::toString() const
	{
		stringstream ss;

		ss << "T [" << getTag() << "], L [" << size() << "]";
		if (currentL > 0)
		{
			ss << ", V " << getValue().toString();
		}

		return ss.str();
	}

	TLVHelper *TLVHelper::getParentTLV()
	{
		return parentTLV;
	}

	bool TLVHelper::enterToValueTLV()
	{
		bool result = false;
		TLVHelper *temp = NULL;

		if (size() >= 2)
		{
			temp = currentTLV->getChildTLV(getValue());

			if (temp != NULL)
			{
				currentTLV = temp;
				result = true;
			}
		}

		return result;
	}

	bool TLVHelper::returnToParentTLV()
	{
		bool result = true;

		if (currentTLV->getParentTLV() != NULL)
		{
			currentTLV = currentTLV->getParentTLV();
		}
		else
		{
			/* top tlv */
		}

		return result;
	}

} /* namespace smartcard_service_api */
