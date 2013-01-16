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


#ifndef PKCS15OBJECT_H_
#define PKCS15OBJECT_H_

/* standard library header */
#include <map>
#include <vector>

/* SLP library header */

/* local header */
#include "FileObject.h"
#include "PKCS15Path.h"

using namespace std;

namespace smartcard_service_api
{
	class PKCS15Object: public FileObject
	{
	protected:
		map<unsigned int, ByteArray> dataList;

	public:
		/* TODO : encapsulate below to asn class */
		static const unsigned int TAG_SEQUENCE = (unsigned int)0x30;
		static const unsigned int TAG_OCTET_STREAM = (unsigned int)0x04;

//		PKCS15Object();
		PKCS15Object(Channel *channel);
		PKCS15Object(Channel *channel, ByteArray selectResponse);
		~PKCS15Object();

		int decodePath(ByteArray path, PKCS15Path &result);
		int getPath(unsigned int type, PKCS15Path &result);
		int getPaths(vector<PKCS15Path> &paths);

		static ByteArray getOctetStream(const ByteArray &data);
	};

} /* namespace smartcard_service_api */
#endif /* PKCS15OBJECT_H_ */
