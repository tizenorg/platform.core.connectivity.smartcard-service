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
#include <list>
#include <string>

/* SLP library header */
#include "dpl/wrt-dao-ro/WrtDatabase.h"
#include "dpl/wrt-dao-ro/widget_dao_read_only.h"
#include "dpl/wrt-dao-ro/wrt_db_types.h"
#include "dpl/db/sql_connection.h"
#include "aul.h"

/* local header */
#include "Debug.h"
#include "SignatureHelper.h"
#include "OpensslHelper.h"

#ifndef EXTERN_API
#define EXTERN_API __attribute__((visibility("default")))
#endif

using namespace WrtDB;
using namespace std;

namespace smartcard_service_api
{
	int SignatureHelper::getProcessName(int pid, char *processName, uint32_t length)
	{
		int ret = -1;
		FILE *file = NULL;
		char filename[1024] = { 0, };

		if (pid < 0 || processName == NULL || length == 0)
			return ret;

		snprintf(filename, sizeof(filename), "/proc/%d/cmdline", pid);
		SCARD_DEBUG("pid : %d, file name : %s", pid, filename);

		if ((file = fopen(filename, "r")) != NULL)
		{
			char *name = NULL;
			ByteArray hash, result;
			size_t len;

			memset(filename, 0, sizeof(filename));
			len = fread(filename, 1, sizeof(filename) - 1, file);
			fclose(file);

			name = basename(filename);
			SCARD_DEBUG("file name : %s", name);

			OpensslHelper::digestBuffer("sha256", (uint8_t *)name, strlen(name), hash);
			SCARD_DEBUG("digest [%d] : %s", hash.getLength(), hash.toString());

			OpensslHelper::encodeBase64String(hash, result, false);

			memset(processName, 0, length);
			memcpy(processName, result.getBuffer(), (result.getLength() < length - 1) ? result.getLength() : length - 1);

			ret = 0;
		}
		else
		{

		}

		return ret;
	}

	ByteArray SignatureHelper::getCertificationHash(const char *packageName)
	{
		ByteArray result;
		list<string>::iterator item;
		CertificateChainList certList;

		SCARD_DEBUG("package name : %s", packageName);

		try
		{
			WrtDatabase::attachToThreadRO();

			int handle = WidgetDAOReadOnly::getHandle(DPL::FromUTF8String(packageName));
			WidgetDAOReadOnly widget(handle);
			certList = widget.getWidgetCertificate();

			SCARD_DEBUG("certList.size [%d]", certList.size());

			WrtDatabase::detachFromThread();
		}
		catch(...)
		{
			SCARD_DEBUG_ERR("exception occurs!!!");
			return result;
		}

		if (certList.size() > 0)
		{
			string certString;
			ByteArray certArray;

#if 0
			for (item = certList.begin(); item != certList.end(); item++)
			{
				SCARD_DEBUG("certList : %s", item->data());
			}
#endif
			certString = certList.back();
			SCARD_DEBUG("certString[%d] :\n%s", certString.size(), certString.data());

			/* base64 decoding */
			if (OpensslHelper::decodeBase64String(certString.data(), certArray) == true)
			{
				int count = 0, offset = 0, length;
//				int i;
				ByteArray cert;

				SCARD_DEBUG("decoded[%d] : %s", certArray.getLength(), certArray.toString());

				/* get count */
				count = *(int *)certArray.getBuffer();
				offset += sizeof(int);
				SCARD_DEBUG("certificate count [%d]", count);

//				for (i = 0; i < count; i++)
				if (count > 0)
				{
					/* certificate length */
					length = *(int *)certArray.getBuffer(offset);
					offset += sizeof(int);
					SCARD_DEBUG("certificate length [%d]", length);

					/* certificate byte stream */
					cert.setBuffer(certArray.getBuffer(offset), length);
					offset += length;

					SCARD_DEBUG("certificate buffer [%d] : %s", cert.getLength(), cert.toString());

					/* sha1 digest */
					if (OpensslHelper::digestBuffer("sha1", cert, result) == true)
					{
						SCARD_DEBUG("digest[%d] : %s", result.getLength(), result.toString());
					}
					else
					{
						SCARD_DEBUG_ERR("digestBuffer failed");
					}
				}
				else
				{
					SCARD_DEBUG_ERR("invalid certificate count [%d]", count);
				}
			}
			else
			{
				SCARD_DEBUG_ERR("decodeBase64String failed");
			}
		}
		else
		{
			SCARD_DEBUG_ERR("certList.size is zero");
		}

		return result;
	}

	ByteArray SignatureHelper::getCertificationHash(int pid)
	{
		ByteArray result;
		int error = 0;
		char pkgName[256] = { 0, };

		if ((error = aul_app_get_pkgname_bypid(pid, pkgName, sizeof(pkgName))) == 0)
		{
			result = getCertificationHash(pkgName);
		}
		else
		{
			SCARD_DEBUG_ERR("aul_app_get_pkgname_bypid failed [%d]", error);
		}

		return result;
	}

} /* namespace smartcard_service_api */

/* export C API */
using namespace smartcard_service_api;

EXTERN_API int signature_helper_get_certificate_hash(const char *packageName, uint8_t *hash, uint32_t *length)
{
	int ret = -1;
	ByteArray result;

	if (packageName == NULL || strlen(packageName) == 0 || hash == NULL || length == NULL || *length < 20)
		return ret;

	result = SignatureHelper::getCertificationHash(packageName);

	if (result.isEmpty() == false)
	{
		memcpy(hash, result.getBuffer(), (result.getLength() < *length) ? result.getLength() : *length);
		*length = result.getLength();

		ret = 0;
	}
	else
	{
		ret = -1;
	}

	return ret;
}

EXTERN_API int signature_helper_get_certificate_hash_by_pid(int pid, uint8_t *hash, uint32_t *length)
{
	int ret = -1;
	ByteArray result;

	if (pid < 0 || hash == NULL || length == NULL || *length < 20)
		return ret;

	result = SignatureHelper::getCertificationHash(pid);

	if (result.isEmpty() == false && result.getLength() < *length)
	{
		memcpy(hash, result.getBuffer(), result.getLength());
		*length = result.getLength();

		ret = 0;
	}
	else
	{
		ret = -1;
	}

	return ret;
}

EXTERN_API int signature_helper_get_process_name(int pid, char *processName, uint32_t length)
{
	int ret = -1;

	if (pid < 0 || processName == NULL || length == 0)
		return ret;

	ret = SignatureHelper::getProcessName(pid, processName, length);

	return ret;
}

