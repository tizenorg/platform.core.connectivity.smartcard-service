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

/* standard library header */
#include <stdio.h>
#include <string.h>
#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>

/* SLP library header */

/* local header */
#include "Debug.h"
#include "ByteArray.h"
#include "OpensslHelper.h"

namespace smartcard_service_api
{
	bool OpensslHelper::encodeBase64String(const ByteArray &buffer, ByteArray &result, bool newLineChar)
	{
		bool ret = false;
		BUF_MEM *bptr;
		BIO *b64, *bmem;

		if (buffer.getLength() == 0)
		{
			return ret;
		}

		b64 = BIO_new(BIO_f_base64());
		bmem = BIO_new(BIO_s_mem());

		if (newLineChar == false)
			BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);

		b64 = BIO_push(b64, bmem);

		BIO_write(b64, buffer.getBuffer(), buffer.getLength());
		BIO_flush(b64);
		BIO_get_mem_ptr(b64, &bptr);

		result.setBuffer((unsigned char *)bptr->data, bptr->length);

		BIO_free_all(b64);

		ret = true;

		return ret;
	}

	bool OpensslHelper::decodeBase64String(const char *buffer, ByteArray &result, bool newLineChar)
	{
		ByteArray temp;

		temp.setBuffer((unsigned char *)buffer, strlen(buffer));

		return decodeBase64String(temp, result, newLineChar);
	}

	bool OpensslHelper::decodeBase64String(const ByteArray &buffer, ByteArray &result, bool newLineChar)
	{
		bool ret = false;
		unsigned int length = 0;
		char *temp;

		if (buffer.getBuffer() == NULL || buffer.getLength() == 0)
		{
			return ret;
		}

		length = buffer.getLength();

		temp = new char[length];
		if (temp != NULL)
		{
			BIO *b64, *bmem;

			memset(temp, 0, length);

			b64 = BIO_new(BIO_f_base64());
			bmem = BIO_new_mem_buf(buffer.getBuffer(), length);
			if (newLineChar == false)
				BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
			bmem = BIO_push(b64, bmem);

			length = BIO_read(bmem, temp, length);

			BIO_free_all(bmem);

			result.setBuffer((unsigned char *)temp, length);

			delete []temp;

			ret = true;
		}
		else
		{
			SCARD_DEBUG_ERR("alloc failed");
		}

		return ret;
	}

	bool OpensslHelper::digestBuffer(const char *algorithm, const uint8_t *buffer, const uint32_t length, ByteArray &result)
	{
		ByteArray temp((uint8_t *)buffer, (uint32_t)length);

		return digestBuffer(algorithm, temp, result);
	}

	bool OpensslHelper::digestBuffer(const char *algorithm, const ByteArray &buffer, ByteArray &result)
	{
		const EVP_MD *md;
		unsigned char *temp;
		bool ret = false;

		if (algorithm == NULL || buffer.getLength() == 0)
		{
			return ret;
		}

		OpenSSL_add_all_digests();

		if ((md = EVP_get_digestbyname(algorithm)) != NULL)
		{
			temp = new unsigned char[EVP_MAX_MD_SIZE];
			if (temp != NULL)
			{
				EVP_MD_CTX mdCtx;
				unsigned int resultLen = 0;

				memset(temp, 0, EVP_MAX_MD_SIZE);

				EVP_DigestInit(&mdCtx, md);
				if (EVP_DigestUpdate(&mdCtx, buffer.getBuffer(), buffer.getLength()) != 0)
				{
					SCARD_DEBUG_ERR("EVP_DigestUpdate failed");
				}
				EVP_DigestFinal(&mdCtx, temp, &resultLen);

				result.setBuffer(temp, resultLen);

				delete []temp;

				ret = true;
			}
			else
			{
				SCARD_DEBUG_ERR("alloc failed");
			}
		}
		else
		{
			SCARD_DEBUG_ERR("EVP_get_digestbyname(\"%s\") returns NULL", algorithm);
		}

		return ret;
	}

} /* namespace smartcard_service_api */
