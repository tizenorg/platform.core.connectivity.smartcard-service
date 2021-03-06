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


#ifndef PKCS15_H_
#define PKCS15_H_

/* standard library header */
#include <map>

/* SLP library header */

/* local header */
#include "PKCS15Object.h"
#include "PKCS15ODF.h"

using namespace std;

namespace smartcard_service_api
{
	class PKCS15: public PKCS15Object
	{
	private:
		map<unsigned int, ByteArray> recordElement;
		PKCS15ODF *odf;

	public:
		static ByteArray PKCS15_AID;

		PKCS15(Channel *channel);
		PKCS15(Channel *channel, ByteArray selectResponse);
		~PKCS15();

		PKCS15ODF *getODF();
		int getTokenInfo(ByteArray &path);
	};

} /* namespace smartcard_service_api */
#endif /* PKCS15_H_ */
