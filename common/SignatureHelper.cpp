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
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <list>
#include <string>
#include <vector>

/* SLP library header */
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
	int SignatureHelper::getPackageName(int pid, char *package, size_t length)
	{
		return aul_app_get_pkgname_bypid(pid, package, length);
	}

	const ByteArray SignatureHelper::getCertificationHash(const char *packageName)
	{
		ByteArray result;
		int ret = 0;
		pkgmgrinfo_certinfo_h handle = NULL;
		pkgmgrinfo_appinfo_h handle_appinfo;
		char *pkgid = NULL;

		if(pkgmgrinfo_appinfo_get_appinfo(packageName, &handle_appinfo) != PMINFO_R_OK)
		{
			_ERR("pkgmgrinfo_appinfo_get_appinfo fail");
			return result;
		}

		if(pkgmgrinfo_appinfo_get_pkgid(handle_appinfo, &pkgid) != PMINFO_R_OK)
		{
			pkgmgrinfo_appinfo_destroy_appinfo(handle_appinfo);
			_ERR("pkgmgrinfo_appinfo_get_pkgid fail");
			return result;
		}
		pkgmgrinfo_appinfo_destroy_appinfo(handle_appinfo);

		SECURE_LOGD("package name : %s, package id : %s", packageName, pkgid);

		if ((ret = pkgmgrinfo_pkginfo_create_certinfo(&handle)) == 0)
		{
			if (0)
			{
				int type;

				for (type = (int)PMINFO_AUTHOR_ROOT_CERT; type <= (int)PMINFO_DISTRIBUTOR2_SIGNER_CERT; type++)
				{
					const char *value = NULL;

					if ((ret = pkgmgrinfo_pkginfo_get_cert_value(handle, (pkgmgrinfo_cert_type)type, &value)) == 0)
					{
						if (value != NULL && strlen(value) > 0)
						{
							OpensslHelper::decodeBase64String(value, result, false);
							if (result.size() > 0)
							{
								_DBG("type [%d] hash [%d] : %s", type, result.size(), result.toString().c_str());
								break;
							}
						}
					}
				}
			}
			else
			{
				_ERR("pkgmgr_pkginfo_load_certinfo failed [%d]", ret);
			}

			pkgmgrinfo_pkginfo_destroy_certinfo(handle);
		}
		else
		{
			_ERR("pkgmgr_pkginfo_create_certinfo failed [%d]", ret);
		}

		return result;
	}

	const ByteArray SignatureHelper::getCertificationHash(int pid)
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
			_ERR("aul_app_get_pkgname_bypid failed [%d]", error);
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
			_ERR("aul_app_get_pkgname_bypid failed [%d]", error);
		}

		return result;
	}

	bool SignatureHelper::getCertificationHashes(const char *packageName, vector<ByteArray> &certHashes)
	{
		bool result = false;
		int ret = 0;
		pkgmgrinfo_certinfo_h handle = NULL;
		pkgmgrinfo_appinfo_h handle_appinfo;
		char *pkgid = NULL;

		if (pkgmgrinfo_appinfo_get_appinfo(packageName, &handle_appinfo) != PMINFO_R_OK)
		{
			_ERR("pkgmgrinfo_appinfo_get_appinfo fail");
			return result;
		}

		if (pkgmgrinfo_appinfo_get_pkgid(handle_appinfo, &pkgid) != PMINFO_R_OK)
		{
			pkgmgrinfo_appinfo_destroy_appinfo(handle_appinfo);
			_ERR("pkgmgrinfo_appinfo_get_pkgid fail");
			return result;
		}

		SECURE_LOGD("package name : %s, package id : %s", packageName, pkgid);

		if ((ret = pkgmgrinfo_pkginfo_create_certinfo(&handle)) == 0)
		{
			if (0)
			{
				int type;

				for (type = (int)PMINFO_AUTHOR_ROOT_CERT; type <= (int)PMINFO_DISTRIBUTOR2_SIGNER_CERT; type++)
				{
					const char *value = NULL;

					if ((ret = pkgmgrinfo_pkginfo_get_cert_value(handle, (pkgmgrinfo_cert_type)type, &value)) == 0)
					{
						if (value != NULL && strlen(value) > 0)
						{
							ByteArray decodeValue, hash;

							OpensslHelper::decodeBase64String(value, decodeValue, false);
							if (decodeValue.size() > 0)
							{
								OpensslHelper::digestBuffer("sha1", decodeValue.getBuffer(), decodeValue.size(), hash);
								if(hash.size() > 0)
								{
									_DBG("type [%d] hash [%d] : %s", type, hash.size(), hash.toString().c_str());
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
				_ERR("pkgmgr_pkginfo_load_certinfo failed [%d]", ret);
			}

			pkgmgrinfo_appinfo_destroy_appinfo(handle_appinfo);

			pkgmgrinfo_pkginfo_destroy_certinfo(handle);
		}
		else
		{
			_ERR("pkgmgr_pkginfo_create_certinfo failed [%d]", ret);
		}

		return result;
	}
} /* namespace smartcard_service_api */

/* export C API */
using namespace smartcard_service_api;

certiHash *__signature_helper_vector_to_linked_list(vector<ByteArray> &certHashes)
{
	vector<ByteArray>::iterator item;
	certiHash *head, *tail;
	uint8_t* buffer = NULL;

	head = tail = NULL;

	for (item = certHashes.begin(); item != certHashes.end(); item++)
	{
		certiHash *tmp = NULL;

		if ((tmp = (certiHash *)calloc(1, sizeof(certiHash))) == NULL)
			goto ERROR;

		tmp->length = (*item).size();

		if ((tmp->value = (uint8_t *)calloc(tmp->length, sizeof(uint8_t))) == NULL)
		{
			free(tmp);
			goto ERROR;
		}

		buffer = (*item).getBuffer();
		if(buffer == NULL)
		{
			free(tmp->value);
			free(tmp);
			continue;
		}

		memcpy(tmp->value, buffer, tmp->length);
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
	_ERR("alloc fail");

	certiHash *tmp;

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

