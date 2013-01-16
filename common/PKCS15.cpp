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

/* SLP library header */

/* local header */
#include "Debug.h"
#include "PKCS15.h"

namespace smartcard_service_api
{
	static unsigned char aid[] = { 0xA0, 0x00, 0x00, 0x00, 0x63, 0x50, 0x4B, 0x43, 0x53, 0x2D, 0x31, 0x35 };
	ByteArray PKCS15::PKCS15_AID(ARRAY_AND_SIZE(aid));

	PKCS15::PKCS15(Channel *channel):PKCS15Object(channel), odf(NULL)
	{
		int ret = 0;

		if ((ret = select(PKCS15::PKCS15_AID)) == 0)
		{
			SCARD_DEBUG("response : %s", selectResponse.toString());
		}
		else
		{
			SCARD_DEBUG_ERR("select failed, [%d]", ret);
		}
	}

	PKCS15::PKCS15(Channel *channel, ByteArray selectResponse):PKCS15Object(channel, selectResponse), odf(NULL)
	{
	}

	PKCS15::~PKCS15()
	{
		if (odf != NULL)
		{
			delete odf;
			odf = NULL;
		}
	}

	PKCS15ODF *PKCS15::getODF()
	{
		if (odf == NULL)
		{
			odf = new PKCS15ODF(channel);

			if (odf != NULL && odf->isClosed() == true)
			{
				SCARD_DEBUG_ERR("failed to open ODF");

				delete odf;
				odf = NULL;
			}
		}

		SCARD_DEBUG("odf [%p]", odf);

		return odf;
	}

} /* namespace smartcard_service_api */
