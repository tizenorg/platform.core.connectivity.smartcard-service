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
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <list>
#include <string>
#include <vector>

/* SLP library header */
#include "package-manager.h"
#include "pkgmgr-info.h"
#include "aul.h"

/* local header */
#include "Debug.h"
#include "SignatureHelper.h"
#include "OpensslHelper.h"

#ifndef EXTERN_API
#define EXTERN_API __attribute__((visibility("default")))
#endif

namespace smartcard_service_api
{
	int SignatureHelper::getProcessName(int pid, char *processName, uint32_t length)
	{
		int ret = -1;
		char buffer[1024] = { 0, };
		char filename[2048] = { 0, };
		int len = 0;

		if (pid < 0 || processName == NULL || length == 0)
			return ret;

		snprintf(buffer, sizeof(buffer), "/proc/%d/exe", pid);
		SCARD_DEBUG("pid : %d, exe : %s", pid, buffer);

		if ((len = readlink(buffer, filename, sizeof(filename) - 1)) < sizeof(filename) - 1)
		{
			char *name = NULL;
			ByteArray hash, result;

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
			SCARD_DEBUG_ERR("readlink failed");
		}

		return ret;
	}

	ByteArray SignatureHelper::getCertificationHash(const char *packageName)
	{
		ByteArray result;
		int ret = 0;
		pkgmgr_certinfo_h handle = NULL;
		pkgmgrinfo_appinfo_h handle_appinfo;
		char *pkgid = NULL;

		if(pkgmgrinfo_appinfo_get_appinfo(packageName, &handle_appinfo) != PMINFO_R_OK)
		{
			SCARD_DEBUG_ERR("pkgmgrinfo_appinfo_get_appinfo fail");
			return result;
		}

		if(pkgmgrinfo_appinfo_get_pkgid(handle_appinfo, &pkgid) != PMINFO_R_OK)
		{
			pkgmgrinfo_appinfo_destroy_appinfo(handle_appinfo);
			SCARD_DEBUG_ERR("pkgmgrinfo_appinfo_get_pkgid fail");
			return result;
		}
		pkgmgrinfo_appinfo_destroy_appinfo(handle_appinfo);

		SCARD_DEBUG("package name : %s", pkgid);

		if ((ret = pkgmgr_pkginfo_create_certinfo(&handle)) == 0)
		{
			if ((ret = pkgmgr_pkginfo_load_certinfo(pkgid, handle)) == 0)
			{
				int type;

				for (type = (int)PM_AUTHOR_ROOT_CERT; type <= (int)PM_DISTRIBUTOR2_SIGNER_CERT; type++)
				{
					const char *value = NULL;

					if ((ret = pkgmgr_pkginfo_get_cert_value(handle, (pkgmgr_cert_type)type, &value)) == 0)
					{
						if (value != NULL && strlen(value) > 0)
						{
							OpensslHelper::decodeBase64String(value, result, false);
							if (result.getLength() > 0)
							{
								SCARD_DEBUG("type [%d] hash [%d] : %s", type, result.getLength(), result.toString());
								break;
							}
						}
					}
				}
			}
			else
			{
				SCARD_DEBUG_ERR("pkgmgr_pkginfo_load_certinfo failed [%d]", ret);
			}

			pkgmgr_pkginfo_destroy_certinfo(handle);
		}
		else
		{
			SCARD_DEBUG_ERR("pkgmgr_pkginfo_create_certinfo failed [%d]", ret);
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

	bool SignatureHelper::getCertificationHashes(int pid, vector<ByteArray> &certHashes)
	{
		bool result = false;
		int error = 0;
		char pkgName[256] = { 0, };

		if ((error = aul_app_get_pkgname_bypid(pid, pkgName, sizeof(pkgName))) == 0)
		{
			result = getCertificationHashes(pkgName, certHashes);
		}
		else
		{
			SCARD_DEBUG_ERR("aul_app_get_pkgname_bypid failed [%d]", error);
		}

		return result;
	}

	bool SignatureHelper::getCertificationHashes(const char *packageName, vector<ByteArray> &certHashes)
	{
		bool result = false;
		int ret = 0;
		pkgmgr_certinfo_h handle = NULL;
		pkgmgrinfo_appinfo_h handle_appinfo;
		char *pkgid = NULL;

		if(pkgmgrinfo_appinfo_get_appinfo(packageName, &handle_appinfo) != PMINFO_R_OK)
		{
			SCARD_DEBUG_ERR("pkgmgrinfo_appinfo_get_appinfo fail");
			return result;
		}

		if(pkgmgrinfo_appinfo_get_pkgid(handle_appinfo, &pkgid) != PMINFO_R_OK)
		{
			pkgmgrinfo_appinfo_destroy_appinfo(handle_appinfo);
			SCARD_DEBUG_ERR("pkgmgrinfo_appinfo_get_pkgid fail");
			return result;
		}
		pkgmgrinfo_appinfo_destroy_appinfo(handle_appinfo);

		SCARD_DEBUG("package name : %s", pkgid);

		if ((ret = pkgmgr_pkginfo_create_certinfo(&handle)) == 0)
		{
			if ((ret = pkgmgr_pkginfo_load_certinfo(pkgid, handle)) == 0)
			{
				int type;

				for (type = (int)PM_AUTHOR_ROOT_CERT; type <= (int)PM_DISTRIBUTOR2_SIGNER_CERT; type++)
				{
					const char *value = NULL;

					if ((ret = pkgmgr_pkginfo_get_cert_value(handle, (pkgmgr_cert_type)type, &value)) == 0)
					{
						if (value != NULL && strlen(value) > 0)
						{
							ByteArray decodeValue, hash;

							OpensslHelper::decodeBase64String(value, decodeValue, false);
							if (decodeValue.getLength() > 0)
							{
								OpensslHelper::digestBuffer("sha1", decodeValue.getBuffer(), decodeValue.getLength(), hash);
								if(hash.getLength() > 0)
								{
									SCARD_DEBUG("type [%d] hash [%d] : %s", type, hash.getLength(), hash.toString());
									certHashes.push_back(hash);
								}
							}
						}
					}
				}

				result = true;
			}
			else
			{
				SCARD_DEBUG_ERR("pkgmgr_pkginfo_load_certinfo failed [%d]", ret);
			}

			pkgmgr_pkginfo_destroy_certinfo(handle);
		}
		else
		{
			SCARD_DEBUG_ERR("pkgmgr_pkginfo_create_certinfo failed [%d]", ret);
		}

		return result;
	}
} /* namespace smartcard_service_api */

/* export C API */
using namespace smartcard_service_api;

certiHash *__signature_helper_vector_to_linked_list(vector<ByteArray> &certHashes)
{
	vector<ByteArray>::iterator item;
	certiHash *head, *tail, *tmp;

	head = tail = NULL;

	for (item = certHashes.begin(); item != certHashes.end(); item++)
	{
		if ((tmp = (certiHash *)calloc(1, sizeof(certiHash))) == NULL)
			goto ERROR;

		tmp->length = (*item).getLength();

		if ((tmp->value = (uint8_t *)calloc(tmp->length, sizeof(uint8_t))) == NULL)
		{
			free(tmp);
			goto ERROR;
		}

		memcpy(tmp->value, (*item).getBuffer(), tmp->length);
		tmp->next = NULL;

		if (head == NULL)
		{
			head = tail = tmp;
		}
		else
		{
			tail->next = tmp;
			tail = tmp;
		}
	}
	return head;

ERROR :
	SCARD_DEBUG_ERR("mem alloc fail");

	while (head)
	{
		tmp = head;
		head = head->next;
		if (tmp->value != NULL)
			free(tmp->value);
		free(tmp);
	}

	return NULL;
}

EXTERN_API int signature_helper_get_process_name(int pid, char *processName, uint32_t length)
{
	int ret = -1;

	if (pid < 0 || processName == NULL || length == 0)
		return ret;

	ret = SignatureHelper::getProcessName(pid, processName, length);

	return ret;
}

EXTERN_API int signature_helper_get_certificate_hashes(const char *packageName, certiHash **hash)
{
	int ret = -1;
	vector<ByteArray> hashes;

	if (packageName == NULL)
		return ret;

	if (SignatureHelper::getCertificationHashes(packageName, hashes) == true)
	{
		*hash = __signature_helper_vector_to_linked_list(hashes);
		ret = 0;
	}

	return ret;
}

EXTERN_API int signature_helper_get_certificate_hashes_by_pid(int pid, certiHash **hash)
{
	int ret = -1;
	vector<ByteArray> hashes;

	if (pid <= 0)
		return ret;

	if (SignatureHelper::getCertificationHashes(pid, hashes) == true)
	{
		*hash = __signature_helper_vector_to_linked_list(hashes);
		ret = 0;
	}

	return ret;
}

