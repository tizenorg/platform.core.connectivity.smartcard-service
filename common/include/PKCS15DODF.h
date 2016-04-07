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

#ifndef PKCS15DODF_H_
#define PKCS15DODF_H_

/* standard library header */

/* SLP library header */

/* local header */
#include "PKCS15Object.h"
#include "PKCS15OID.h"

#ifndef LIBSCL_EXPORT_API
#define LIBSCL_EXPORT_API
#endif // LIBSCL_EXPORT_API


namespace smartcard_service_api
{
	class LIBSCL_EXPORT_API PKCS15DODF : public PKCS15Object
	{
	private:
		map<ByteArray, PKCS15OID> mapOID;

		bool parseData(const ByteArray &data);

	public:
		PKCS15DODF(unsigned int fid, Channel *channel);
		PKCS15DODF(const ByteArray &path, Channel *channel);
		~PKCS15DODF();

		int searchOID(const ByteArray &oid, ByteArray &data) const;
	};

} /* namespace smartcard_service_api */
#endif /* PKCS15DODF_H_ */
