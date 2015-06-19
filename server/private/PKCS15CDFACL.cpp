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
#include "PKCS15CDFACL.h"
#include "PKCS15ODF.h"
#include "OpensslHelper.h"
#include "AccessCondition.h"

#ifndef EXTERN_API
#define EXTERN_API __attribute__((visibility("default")))
#endif

namespace smartcard_service_api
{
	PKCS15CDFACL::PKCS15CDFACL() : AccessControlList()
	{
	}

	PKCS15CDFACL::~PKCS15CDFACL()
	{
	}

	int PKCS15CDFACL::loadACL(Channel *channel)
	{
		int result = SCARD_ERROR_OK;

		_BEGIN();

		if (channel == NULL)
		{
			return SCARD_ERROR_ILLEGAL_PARAM;
		}

		releaseACL();

		PKCS15 pkcs15(channel);

		result = pkcs15.select();
		if (result >= SCARD_ERROR_OK)
		{
			PKCS15ODF *odf;

			result = SCARD_ERROR_OK;

			if ((odf = pkcs15.getODF()) != NULL)
			{
				PKCS15CDF *cdf;

				cdf = odf->getCDF();
				if (cdf != NULL)
				{
					result = loadRules(channel, cdf);
					if (result == SCARD_ERROR_OK)
					{
						printAccessControlList();
					}
					else
					{
						result = SCARD_ERROR_OK;
					}
				}
				else
				{
					_ERR("dodf null, every request will be denied.");
				}
			}
			else
			{
				_ERR("odf null, every request will be denied.");
			}
		}
		else
		{
			_ERR("failed to open PKCS#15, every request will be denied.");
		}

		_END();

		return result;
	}

	int PKCS15CDFACL::loadRules(Channel *channel, PKCS15CDF *cdf)
	{
		int result = 0;
		size_t i;
		ByteArray hash;
		const CertificateType *type;
		AccessCondition condition;

		condition.setAID(AccessControlList::ALL_SE_APPS);

		for (i = 0; i < cdf->getCount(); i++) {
			type = cdf->getCertificateType(i);
			if(type == NULL)
				continue;

			OpensslHelper::digestBuffer("SHA1", type->certificate, hash);

			_INFO("cdf[%d] = %s", i, hash.toString().c_str());

			condition.addAccessRule(hash);
		}

		pair<ByteArray, AccessCondition> newItem(
			AccessControlList::ALL_SE_APPS, condition);

		mapConditions.insert(newItem);

		return result;
	}
} /* namespace smartcard_service_api */
