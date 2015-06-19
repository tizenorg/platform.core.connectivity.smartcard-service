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

#ifndef GPARFACL_H_
#define GPARFACL_H_

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
	class GPARFACL : public AccessControlList
	{
	private:
		ByteArray refreshTag;

		int loadAccessControl(Channel *channel, PKCS15DODF *dodf);
		int loadRules(Channel *channel, const ByteArray &path);
		int loadAccessConditions(Channel *channel, const ByteArray &aid, const ByteArray &path);

	public:
		GPARFACL();
		~GPARFACL();

		int loadACL(Channel *channel);
	};

} /* namespace smartcard_service_api */
#endif /* __cplusplus */
#endif /* GPARFACL_H_ */
