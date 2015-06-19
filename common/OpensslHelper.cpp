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

		if (buffer.size() == 0)
		{
			return ret;
		}

		b64 = BIO_new(BIO_f_base64());
		if(b64 == NULL)
			return false;

		bmem = BIO_new(BIO_s_mem());

		if (newLineChar == false)
			BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);

		b64 = BIO_push(b64, bmem);

		BIO_write(b64, buffer.getBuffer(), buffer.size());
		BIO_flush(b64);
		BIO_get_mem_ptr(b64, &bptr);

		result.assign((unsigned char *)bptr->data, bptr->length);

		BIO_free_all(b64);

		ret = true;

		return ret;
	}

	bool OpensslHelper::decodeBase64String(const char *buffer, ByteArray &result, bool newLineChar)
	{
		ByteArray temp;

		temp.assign((unsigned char *)buffer, strlen(buffer));

		return decodeBase64String(temp, result, newLineChar);
	}

	bool OpensslHelper::decodeBase64String(const ByteArray &buffer, ByteArray &result, bool newLineChar)
	{
		bool ret = false;
		unsigned int length = 0;
		char *temp;

		if (buffer.getBuffer() == NULL || buffer.size() == 0)
		{
			return ret;
		}

		length = buffer.size();

		temp = new char[length];
		if (temp != NULL)
		{
			BIO *b64, *bmem;

			memset(temp, 0, length);

			b64 = BIO_new(BIO_f_base64());
			if(b64 == NULL)
			{
				delete []temp;
				return false;
			}

			bmem = BIO_new_mem_buf((void *)buffer.getBuffer(), length);
			if (newLineChar == false)
				BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
			bmem = BIO_push(b64, bmem);

			length = BIO_read(bmem, temp, length);

			BIO_free_all(bmem);
			if(length > 0)
			{
				result.assign((unsigned char *)temp, length);
				ret = true;
			}
			else
			{
				ret = false;
			}

			delete []temp;
		}
		else
		{
			_ERR("alloc failed");
		}

		return ret;
	}

	bool OpensslHelper::digestBuffer(const char *algorithm,
		const uint8_t *buffer, uint32_t length, ByteArray &result)
	{
		ByteArray temp(buffer, length);

		return digestBuffer(algorithm, temp, result);
	}

	bool OpensslHelper::digestBuffer(const char *algorithm,
		const ByteArray &buffer, ByteArray &result)
	{
		const EVP_MD *md;
		bool ret = false;

		if (algorithm == NULL || buffer.size() == 0)
		{
			return ret;
		}

		OpenSSL_add_all_digests();

		if ((md = EVP_get_digestbyname(algorithm)) != NULL)
		{
			uint8_t temp[EVP_MAX_MD_SIZE] = { 0, };
			EVP_MD_CTX mdCtx;
			unsigned int resultLen = 0;

			if (EVP_DigestInit(&mdCtx, md) > 0)
			{
				if (EVP_DigestUpdate(&mdCtx, buffer.getBuffer(), buffer.size()) == 0)
				{
					_ERR("EVP_DigestUpdate failed");
				}

				if (EVP_DigestFinal(&mdCtx, temp, &resultLen) > 0 &&
					resultLen > 0)
				{
					result.assign(temp, resultLen);
					ret = true;
				}
			}
		}
		else
		{
			_ERR("EVP_get_digestbyname(\"%s\") returns NULL", algorithm);
		}

		return ret;
	}

} /* namespace smartcard_service_api */
