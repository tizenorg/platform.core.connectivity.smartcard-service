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

/* SLP library header */

/* local header */
#include "Debug.h"
#include "TLVHelper.h"

namespace smartcard_service_api
{
	void TLVHelper::initialize(TLVHelper *parent)
	{
		parentTLV = parent;
		childTLV = NULL;
		currentTLV = this;
		offset = 0;
		currentT = 0;
		currentL = 0;
	}

	TLVHelper::TLVHelper()
	{
		initialize();
	}

	TLVHelper::TLVHelper(TLVHelper *parent)
	{
		initialize(parent);
	}

	TLVHelper::TLVHelper(const ByteArray &array)
	{
		setTLVBuffer(array);
	}

	TLVHelper::TLVHelper(const ByteArray &array, TLVHelper *parent)
	{
		setTLVBuffer(array, parent);
	}

	TLVHelper::~TLVHelper()
	{
	}

	bool TLVHelper::setTLVBuffer(const ByteArray &array, TLVHelper *parent)
	{
		initialize(parent);

		if (array.getLength() == 0)
			return false;

		tlvBuffer = array;

		return true;
	}

	bool TLVHelper::setTLVBuffer(unsigned char *buffer, unsigned int length, TLVHelper *parent)
	{
		return setTLVBuffer(ByteArray(buffer, length), parent);
	}

	bool TLVHelper::_decodeTLV()
	{
		int result;

		currentT = 0;
		currentL = 0;
		currentV.releaseBuffer();

		if (isEndOfBuffer())
			return false;

		/* T */
		if ((result = decodeTag(tlvBuffer.getBuffer(offset))) < 0)
			return false;

		offset += result;

		/* L */
		if ((result = decodeLength(tlvBuffer.getBuffer(offset))) < 0)
			return false;

		offset += result;

		if (currentL > 0)
		{
			/* V */
			if ((result = decodeValue(tlvBuffer.getBuffer(offset))) < 0)
				return false;

			offset += result;
		}

		return true;
	}

	const char *TLVHelper::toString()
	{
		memset(strBuffer, 0, sizeof(strBuffer));

		if (currentL == 0)
		{
			snprintf(strBuffer, sizeof(strBuffer), "T [%X], L [%d]", getTag(), getLength());
		}
		else
		{
			snprintf(strBuffer, sizeof(strBuffer), "T [%X], L [%d], V %s", getTag(), getLength(), getValue().toString());
		}

		return strBuffer;
	}

	TLVHelper *TLVHelper::getParentTLV()
	{
		return parentTLV;
	}

	bool TLVHelper::enterToValueTLV()
	{
		bool result = false;
		TLVHelper *temp = NULL;

		if (getLength() >= 2)
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

//		SCARD_DEBUG("current [%p], parent [%p]", currentTLV, currentTLV->getParentTLV());

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
