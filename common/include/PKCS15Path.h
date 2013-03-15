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

#ifndef PKCS15Path_H_
#define PKCS15Path_H_

/* standard library header */

/* SLP library header */

/* local header */
#include "ByteArray.h"

namespace smartcard_service_api
{
	class PKCS15Path
	{
	private :
		ByteArray path;
		int index;
		int length;

		bool parseData(ByteArray &data);

	public:
		PKCS15Path();
		PKCS15Path(ByteArray &data);
		PKCS15Path(ByteArray path, int index);
		PKCS15Path(unsigned char *path, unsigned int length, int index);
		~PKCS15Path();

		int getPath(ByteArray &path);
		bool hasIndexLength();
		int getIndex();
		unsigned int getLength();
		int encode(ByteArray &result);
	};

} /* namespace smartcard_service_api */
#endif /* PKCS15Path_H_ */
