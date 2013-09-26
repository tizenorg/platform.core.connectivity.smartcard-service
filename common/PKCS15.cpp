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

#include "Debug.h"
#include "APDUHelper.h"
#include "EFDIR.h"
#include "PKCS15.h"

namespace smartcard_service_api
{
	static unsigned char aid[] = { 0xA0, 0x00, 0x00, 0x00, 0x63, 0x50,
		0x4B, 0x43, 0x53, 0x2D, 0x31, 0x35 };
	ByteArray PKCS15::PKCS15_AID(ARRAY_AND_SIZE(aid));

	PKCS15::PKCS15(Channel *channel) :
		PKCS15Object(channel), odf(NULL)
	{
	}

	PKCS15::PKCS15(Channel *channel, const ByteArray &selectResponse) :
		PKCS15Object(channel, selectResponse), odf(NULL)
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

	int PKCS15::select()
	{
		int ret;

		ret = PKCS15Object::select(PKCS15::PKCS15_AID);
		if (ret >= SCARD_ERROR_OK)
		{
			_DBG("response : %s", selectResponse.toString().c_str());
		}
		else if (ret == ResponseHelper::ERROR_FILE_NOT_FOUND)
		{
			_ERR("PKCS15 AID not found, search in EF DIR");

			ret = selectFromEFDIR();
			if (ret >= SCARD_ERROR_OK)
			{
				_DBG("response : %s", selectResponse.toString().c_str());
			}
			else
			{
				_ERR("PKCS15 select failed, [%d]", ret);
			}
		}
		else
		{
			_ERR("PKCS15 select failed, [%d]", ret);
		}

		return ret;
	}

	int PKCS15::selectFromEFDIR()
	{
		int ret;
		ByteArray path;
		EFDIR dir(channel);

		ret = dir.select();
		if (ret >= SCARD_ERROR_OK)
		{
			path = dir.getPathByAID(PKCS15_AID);
			if (path.size() > 0)
			{
				ret = PKCS15Object::select(path, false);
				if (ret < SCARD_ERROR_OK)
				{
					_ERR("PKCS15 select failed, [%d]", ret);
				}
			}
			else
			{
				_ERR("PKCS15 path is not found");
				ret = SCARD_ERROR_NOT_SUPPORTED;
			}
		}
		else
		{
			_ERR("select EFDIR failed, [%x]", -ret);
		}

		return ret;
	}

	PKCS15ODF *PKCS15::getODF()
	{
		if (odf == NULL)
		{
			odf = new PKCS15ODF(channel);

			if (odf != NULL && odf->isClosed() == true)
			{
				_ERR("failed to open ODF");

				delete odf;
				odf = NULL;
			}
		}

		_DBG("odf [%p]", odf);

		return odf;
	}
} /* namespace smartcard_service_api */
