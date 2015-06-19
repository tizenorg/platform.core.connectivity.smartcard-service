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

#ifndef PKCS15CDFACL_H_
#define PKCS15CDFACL_H_

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
	class PKCS15CDFACL : public AccessControlList
	{
	private:
		int loadRules(Channel *channel, PKCS15CDF *cdf);

	public:
		PKCS15CDFACL();
		~PKCS15CDFACL();

		int loadACL(Channel *channel);
	};

} /* namespace smartcard_service_api */
#endif /* __cplusplus */
#endif /* PKCS15CDFACL_H_ */
