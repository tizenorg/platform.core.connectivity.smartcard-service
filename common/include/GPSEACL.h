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


#ifndef GPSEACL_H_
#define GPSEACL_H_

/* standard library header */

/* SLP library header */

/* local header */
#include "smartcard-types.h"
#ifdef __cplusplus
#include "AccessControlList.h"
#include "PKCS15.h"
#endif /* __cplusplus */

#ifdef __cplusplus
namespace smartcard_service_api
{
	class GPSEACL: public AccessControlList
	{
	private:
		PKCS15 *pkcs15;
		ByteArray refreshTag;

		static ByteArray OID_GLOBALPLATFORM;

		int loadAccessControl(PKCS15DODF *dodf);
		int loadRules(ByteArray path);
		int loadAccessConditions(ByteArray aid, ByteArray path);

	public:
		GPSEACL(Channel *channel);
		~GPSEACL();

		int loadACL();

	};

} /* namespace smartcard_service_api */
#endif /* __cplusplus */

/* export C API */
#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

typedef void *gp_se_acl_h;

gp_se_acl_h gp_se_acl_create_instance(channel_h channel);
int gp_se_acl_load_acl(gp_se_acl_h handle);
int gp_se_acl_update_acl(gp_se_acl_h handle);
void gp_se_acl_release_acl(gp_se_acl_h handle);
bool gp_se_acl_is_authorized_access(gp_se_acl_h handle, unsigned char *aidBuffer, unsigned int aidLength, unsigned char *certHashBuffer, unsigned int certHashLength);
void gp_se_acl_destroy_instance(gp_se_acl_h handle);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* GPSEACL_H_ */
