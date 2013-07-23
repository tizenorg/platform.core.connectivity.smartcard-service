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

#ifdef USE_GDBUS
/* standard library header */
#include <glib.h>

/* SLP library header */
#ifdef USER_SPACE_SMACK
#include "security-server.h"
#endif

/* local header */
#include "smartcard-types.h"
#include "Debug.h"
#include "ByteArray.h"
#include "ClientGDBus.h"

using namespace std;

namespace smartcard_service_api
{
	ByteArray ClientGDBus::cookie = ByteArray::EMPTY;

	GVariant *ClientGDBus::getCookie()
	{
		GVariant *result;
#ifdef USER_SPACE_SMACK
		if (cookie.isEmpty()) {
			uint8_t *buffer;
			int len;

			len = security_server_get_cookie_size();
			if (len > 0) {
				buffer = new uint8_t[len];
				if (buffer != NULL) {
					if (security_server_request_cookie(
						(char *)buffer, len) == 0) {
						cookie.assign(buffer, len);
					} else {
						_ERR("security_server_request_cookie failed");
					}

					delete[] buffer;
				} else {
					_ERR("alloc failed");
				}
			} else {
				_ERR("security_server_get_cookie_size failed");
			}
		}
#endif
		result = GDBusHelper::convertByteArrayToVariant(cookie);

		return result;
	}
} /* namespace smartcard_service_api */
#endif
