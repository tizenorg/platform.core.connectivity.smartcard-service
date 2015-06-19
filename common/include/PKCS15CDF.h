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

#ifndef PKCS15CDF_H_
#define PKCS15CDF_H_

/* standard library header */

/* SLP library header */

/* local header */
#include "PKCS15Object.h"
#include "PKCS15OID.h"

namespace smartcard_service_api
{
	class CertificateType
	{
	public :
		/* Common Object Attributes */
		string label;
		bool modifiable;

		/* Common Certificate Attributes */
		ByteArray id;
		int authority;

		/* Certificate Attributes */
		ByteArray path;
		int index;
		size_t length;
		ByteArray certificate;
	};

	class PKCS15CDF : public PKCS15Object
	{
	private:
		vector<CertificateType *> listCertType;

		bool parseData(const ByteArray &data);

	public:
		PKCS15CDF(unsigned int fid, Channel *channel);
		PKCS15CDF(const ByteArray &path, Channel *channel);
		~PKCS15CDF();

		inline size_t getCount() const { return listCertType.size(); }
		const CertificateType *getCertificateType(int index) const;
	};

} /* namespace smartcard_service_api */
#endif /* PKCS15DODF_H_ */
