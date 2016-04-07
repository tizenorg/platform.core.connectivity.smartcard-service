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

#ifndef SIGNATUREHELPER_H_
#define SIGNATUREHELPER_H_

/* standard library header */
#ifdef __cplusplus
#include <vector>
#endif /* __cplusplus */

/* SLP library header */

/* local header */
#include "smartcard-types.h"
#ifdef __cplusplus
#include "ByteArray.h"
#endif /* __cplusplus */

#ifdef __cplusplus

#ifndef LIBSCL_EXPORT_API
#define LIBSCL_EXPORT_API
#endif // LIBSCL_EXPORT_API

using namespace std;

namespace smartcard_service_api
{
	class LIBSCL_EXPORT_API SignatureHelper
	{
	public:
		static int getPackageName(int pid, char *package, size_t length);
		static const ByteArray getCertificationHash(const char *packageName);
		static const ByteArray getCertificationHash(int pid);
		static bool getCertificationHashes(int pid, vector<ByteArray> &certHashes);
		static bool getCertificationHashes(const char *packageName, vector<ByteArray> &certHashes);
	};

} /* namespace smartcard_service_api */
#endif /* __cplusplus */

/* export C API */
#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

typedef struct LIBSCL_EXPORT_API _certiHash
{
	uint8_t *value;
	uint32_t length;
	struct _certiHash *next;
} certiHash;

typedef void (*signature_helper_get_certificate_hashes_cb)(void *user_param, uint8_t *hash, uint32_t length);

LIBSCL_EXPORT_API int signature_helper_get_certificate_hashes(const char *packageName, certiHash **hash);
LIBSCL_EXPORT_API int signature_helper_get_certificate_hashes_by_pid(int pid, certiHash **hash);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* SIGNATUREHELPER_H_ */
