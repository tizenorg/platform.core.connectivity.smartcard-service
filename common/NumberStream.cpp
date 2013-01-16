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


#include "NumberStream.h"

namespace smartcard_service_api
{
	NumberStream::NumberStream(const ByteArray &T)
	{
		setBuffer(T.getBuffer(), T.getLength());
	}

	unsigned int NumberStream::getBigEndianNumber()
	{
		return getBigEndianNumber(*this);
	}

	unsigned int NumberStream::getLittleEndianNumber()
	{
		return getLittleEndianNumber(*this);
	}

	NumberStream &NumberStream::operator =(const ByteArray &T)
	{
		if (this != &T)
		{
			setBuffer(T.getBuffer(), T.getLength());
		}

		return *this;
	}

	NumberStream &NumberStream::operator =(const NumberStream &T)
	{
		if (this != &T)
		{
			setBuffer(T.getBuffer(), T.getLength());
		}

		return *this;
	}

	unsigned int NumberStream::getBigEndianNumber(const ByteArray &T)
	{
		int i, len;
		unsigned int result = 0;

		len = (T.getLength() < 4) ? T.getLength() : 4;

		for (i = 0; i < len; i++)
		{
			result = (result << 8) | T.getAt(i);
		}

		return result;
	}

	unsigned int NumberStream::getLittleEndianNumber(const ByteArray &T)
	{
		int i, len;
		unsigned int result = 0;

		len = (T.getLength() < 4) ? T.getLength() : 4;

		for (i = 0; i < len; i++)
		{
			result = result | (T.getAt(i) << (i * 8));
		}

		return result;
	}

} /* namespace smartcard_service_api */
