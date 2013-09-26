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

#include "Debug.h"
#include "GPACE.h"
#include "GPARAACL.h"
#include "GPARFACL.h"
#include "SessionHelper.h"
#include "ReaderHelper.h"

#ifndef EXTERN_API
#define EXTERN_API __attribute__((visibility("default")))
#endif

namespace smartcard_service_api
{
	GPACE::GPACE() : AccessControlList(), acl(NULL)
	{
	}

	GPACE::~GPACE()
	{
		if (acl != NULL) {
			delete acl;
		}
	}

	int GPACE::loadACL(Channel *channel)
	{
		int result = SCARD_ERROR_OK;

		_BEGIN();

		if (channel == NULL)
		{
			return SCARD_ERROR_ILLEGAL_PARAM;
		}

		if (acl == NULL) {
			/* first, check ara-m */
			GPARAACL *araACL = new GPARAACL;

			result = araACL->loadACL(channel);
			if (result < SCARD_ERROR_OK) {
				_ERR("ARA not found");

				delete araACL;

				if (true) {
					_INFO("try to use ARF");
					/* second, check arf when channel is for SIM */
					GPARFACL *arfACL = new GPARFACL;

					result = arfACL->loadACL(channel);
					if (result >= SCARD_ERROR_OK) {
						acl = arfACL;
					} else {
						delete arfACL;
					}
				}
			} else {
				acl = araACL;
			}
		} else {
			result = acl->loadACL(channel);
		}

		_END();

		return result;
	}

	bool GPACE::isAuthorizedAccess(const ByteArray &aid,
		const ByteArray &certHash) const
	{
		return (acl != NULL) ? acl->isAuthorizedAccess(aid, certHash) : false;
	}

	bool GPACE::isAuthorizedAccess(const unsigned char *aidBuffer,
		unsigned int aidLength, const unsigned char *certHashBuffer,
		unsigned int certHashLength) const
	{
		return (acl != NULL) ? acl->isAuthorizedAccess(aidBuffer, aidLength, certHashBuffer, certHashLength) : false;
	}

	bool GPACE::isAuthorizedAccess(const ByteArray &aid,
		const vector<ByteArray> &certHashes) const
	{
		return (acl != NULL) ? acl->isAuthorizedAccess(aid, certHashes) : false;
	}

	bool GPACE::isAuthorizedAccess(const ByteArray &aid,
		const vector<ByteArray> &certHashes,
		const ByteArray &command) const
	{
		return (acl != NULL) ? acl->isAuthorizedAccess(aid, certHashes, command) : false;
	}

	bool GPACE::isAuthorizedNFCAccess(const ByteArray &aid,
		const vector<ByteArray> &certHashes) const
	{
		return (acl != NULL) ? acl->isAuthorizedNFCAccess(aid, certHashes) : false;
	}

} /* namespace smartcard_service_api */
