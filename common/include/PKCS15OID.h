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

#ifndef PKCS15OID_H_
#define PKCS15OID_H_

/* standard library header */

/* SLP library header */

/* local header */
#include "ByteArray.h"

namespace smartcard_service_api
{
	class PKCS15OID
	{
	private:
		ByteArray oid;
		ByteArray name;
		ByteArray path;

		bool parseOID(ByteArray data);

	public:
		PKCS15OID(ByteArray data);
		~PKCS15OID();

		ByteArray getOID();
		ByteArray getName();
		ByteArray getPath();

	};

} /* namespace smartcard_service_api */
#endif /* PKCS15OID_H_ */
