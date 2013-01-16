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
#include <stdint.h>

/* SLP library header */

/* local header */
#include "ByteArray.h"

#ifndef OPENSSLHELPER_H_
#define OPENSSLHELPER_H_

namespace smartcard_service_api
{
	class OpensslHelper
	{
	public:
		/* base64 method */
		static bool encodeBase64String(const ByteArray &buffer, ByteArray &result, bool newLineChar = false);
		static bool decodeBase64String(const char *buffer, ByteArray &result, bool newLineChar = true);
		static bool decodeBase64String(const ByteArray &buffer, ByteArray &result, bool newLineChar = true);

		/* digest method */
		static bool digestBuffer(const char *algorithm, const uint8_t *buffer, const uint32_t length, ByteArray &result);
		static bool digestBuffer(const char *algorithm, const ByteArray &buffer, ByteArray &result);
	};

} /* namespace smartcard_service_api */
#endif /* OPENSSLHELPER_H_ */
