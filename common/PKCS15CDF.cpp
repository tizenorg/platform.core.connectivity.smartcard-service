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

/* standard library header */

/* SLP library header */

/* local header */
#include "Debug.h"
#include "NumberStream.h"
#include "PKCS15CDF.h"
#include "SimpleTLV.h"

namespace smartcard_service_api
{
	PKCS15CDF::PKCS15CDF(unsigned int fid, Channel *channel) :
		PKCS15Object(channel)
	{
		int ret = 0;

		if ((ret = select(fid)) >= SCARD_ERROR_OK)
		{
			ByteArray cdfData, extra;

			_DBG("response : %s", selectResponse.toString().c_str());

			ret = readBinaryAll(0, cdfData);
			if (ret == SCARD_ERROR_OK)
			{
				_DBG("cdfData : %s", cdfData.toString().c_str());

				parseData(cdfData);
			}
			else
			{
				_ERR("readBinary failed, [%d]", ret);
			}
		}
		else
		{
			_ERR("select failed, [%d]", ret);
		}
	}

	PKCS15CDF::PKCS15CDF(const ByteArray &path, Channel *channel) :
		PKCS15Object(channel)
	{
		int ret = 0;

		if ((ret = select(path)) >= SCARD_ERROR_OK)
		{
			ByteArray cdfData, extra;

			_DBG("response : %s", selectResponse.toString().c_str());

			ret = readBinaryAll(0, cdfData);
			if (ret == SCARD_ERROR_OK)
			{
				_DBG("cdfData : %s", cdfData.toString().c_str());

				parseData(cdfData);
			}
			else
			{
				_ERR("readBinary failed, [%d]", ret);
			}
		}
		else
		{
			_ERR("select failed, [%d]", ret);
		}
	}

	PKCS15CDF::~PKCS15CDF()
	{
	}

	bool PKCS15CDF::parseData(const ByteArray &data)
	{
		int result;
		char* buffer;
		SimpleTLV tlv(data);

		while (tlv.decodeTLV())
		{
			CertificateType *cert;

			_DBG("0x%02X [%d] : %s", tlv.getTag(), tlv.size(), tlv.getValue().toString().c_str());

			cert = new CertificateType();

			tlv.enterToValueTLV();
			if (tlv.decodeTLV())
			{
				_DBG("Common Object Attributes");

				/* Common Object Attributes */
				tlv.enterToValueTLV();
				while (tlv.decodeTLV())
				{
					switch (tlv.getTag())
					{
					case (unsigned int)0x0C : /* label : OCTET STRING */
						buffer = (char *)tlv.getValue().getBuffer();
						if(buffer != NULL)
						{
							_DBG("label : %s", buffer);
							cert->label.assign(buffer, tlv.getValue().getLength());
						}
						break;

					case (unsigned int)0x03 : /* flags : BIT STRING */
						/* 0 : private, 1 : modifiable */
						_DBG("flag : %s", tlv.getValue()[0] ? "modifiable" : "private");
						cert->modifiable = (tlv.getValue()[0] == 1);
						break;

					default :
						_DBG("0x%02X [%d] : %s", tlv.getTag(), tlv.size(), tlv.getValue().toString().c_str());
						break;
					}
				}
				tlv.returnToParentTLV();
			}

			if (tlv.decodeTLV())
			{
				_DBG("Common Certificate Attributes");

				/* Common Certificate Attributes */
				tlv.enterToValueTLV();
				while (tlv.decodeTLV())
				{
					switch (tlv.getTag())
					{
					case (unsigned int)0x04 : /* iD : OCTET STRING */
						_DBG("id : %s", tlv.getValue().toString().c_str());
						cert->id = tlv.getValue();
						break;

					case (unsigned int)0x01 : /* Authority : BOOLEAN */
						_DBG("authority : %s", tlv.getValue().toString().c_str());
						cert->authority = tlv.getValue()[0];
						break;

					case (unsigned int)0xA1 : /* ??? : ??? */
						tlv.enterToValueTLV();
						if (tlv.decodeTLV()) {
							_DBG("    0x%02X [%d] : %s", tlv.getTag(), tlv.size(), tlv.getValue().toString().c_str());
							tlv.enterToValueTLV();
							if (tlv.decodeTLV()) {
								_DBG("      0x%02X [%d] : %s", tlv.getTag(), tlv.size(), tlv.getValue().toString().c_str());
								tlv.enterToValueTLV();
								if (tlv.decodeTLV()) {
									_DBG("        0x%02X [%d] : %s", tlv.getTag(), tlv.size(), tlv.getValue().toString().c_str());
									tlv.enterToValueTLV();
									if (tlv.decodeTLV()) {
										_DBG("          0x%02X [%d] : %s", tlv.getTag(), tlv.size(), tlv.getValue().toString().c_str());
									}
									tlv.returnToParentTLV();
								}
								if (tlv.decodeTLV()) {
									_DBG("        0x%02X [%d] : %s", tlv.getTag(), tlv.size(), tlv.getValue().toString().c_str());
								}
								tlv.returnToParentTLV();
							}
							tlv.returnToParentTLV();
						}
						tlv.returnToParentTLV();
						break;

					default :
						_DBG("0x%02X [%d] : %s", tlv.getTag(), tlv.size(), tlv.getValue().toString().c_str());
						break;
					}
				}
				tlv.returnToParentTLV();
			}

			if (tlv.decodeTLV())
			{
				_DBG("Certificate Attributes");

				/* Path or Object */
				tlv.enterToValueTLV();
				if (tlv.decodeTLV())
				{
					tlv.enterToValueTLV();
					if (tlv.decodeTLV())
					{
						/* PATH */
						tlv.enterToValueTLV();
						while (tlv.decodeTLV())
						{
							switch (tlv.getTag())
							{
							case (unsigned int)0x04 : /* path : OCTET STRING */
								cert->path = tlv.getValue();
								_DBG("path : %s", cert->path.toString().c_str());
								break;

							case (unsigned int)0x02 : /* index : INTEGER */
								cert->index = NumberStream::getBigEndianNumber(tlv.getValue());
								_DBG("index : %d", cert->index);
								break;

							case (unsigned int)0x80 : /* length : INTEGER */
								cert->length = NumberStream::getBigEndianNumber(tlv.getValue());
								_DBG("length : %d", cert->length);
								break;
							}
						}
						tlv.returnToParentTLV();

						FileObject file(channel);

						result = file.select(cert->path, true);
						if (result >= SCARD_ERROR_OK) {
							result = file.readBinary(0, cert->length, cert->certificate);
							if (result >= SCARD_ERROR_OK) {
								_DBG("certificate[%d] : %s", cert->certificate.size(), cert->certificate.toString().c_str());
							} else {
								_ERR("readBinary failed, [%x]", result);
							}
						} else {
							_ERR("select failed, [%x]", result);
						}

					}
					tlv.returnToParentTLV();
				}
				tlv.returnToParentTLV();
			}
			tlv.returnToParentTLV();

			listCertType.push_back(cert);
		}

		_INFO("listCertType.size() = %d", listCertType.size());

		return (listCertType.size() > 0);
	}

	const CertificateType *PKCS15CDF::getCertificateType(int index) const
	{
		if (index < 0 || index >= (int)listCertType.size())
			return NULL;

		return listCertType[index];
	}
} /* namespace smartcard_service_api */
