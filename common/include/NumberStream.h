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

#ifndef NUMBERSTREAM_H_
#define NUMBERSTREAM_H_

#include "ByteArray.h"

namespace smartcard_service_api
{
	class NumberStream : public ByteArray
	{
	public:
		NumberStream(const ByteArray &T);

		unsigned int getBigEndianNumber() const;
		unsigned int getLittleEndianNumber() const;

		static unsigned int getBigEndianNumber(const ByteArray &T);
		static unsigned int getLittleEndianNumber(const ByteArray &T);

		NumberStream &operator =(const ByteArray &T);
		NumberStream &operator =(const NumberStream &T);
	};

} /* namespace smartcard_service_api */
#endif /* NUMBERSTREAM_H_ */
