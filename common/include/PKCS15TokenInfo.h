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

#ifndef PKCS15TOKENINFO_H_
#define PKCS15TOKENINFO_H_

/* standard library header */

/* SLP library header */

/* local header */
#include "PKCS15Object.h"

#ifndef LIBSCL_EXPORT_API
#define LIBSCL_EXPORT_API
#endif // LIBSCL_EXPORT_API


namespace smartcard_service_api
{
	class LIBSCL_EXPORT_API PKCS15TokenInfo : public PKCS15Object
	{
	public:
		PKCS15TokenInfo(Channel *channel);
		~PKCS15TokenInfo();
	};

} /* namespace smartcard_service_api */
#endif /* PKCS15TOKENINFO_H_ */
