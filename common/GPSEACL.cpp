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
#include "GPSEACL.h"
#include "PKCS15ODF.h"
#include "PKCS15DODF.h"
#include "NumberStream.h"
#include "SimpleTLV.h"
#include "AccessCondition.h"

#ifndef EXTERN_API
#define EXTERN_API __attribute__((visibility("default")))
#endif

namespace smartcard_service_api
{
	static unsigned char oid_globalplatform[] = { 0x2A, 0x86, 0x48, 0x86, 0xFC, 0x6B, 0x81, 0x48, 0x01, 0x01 };
	ByteArray GPSEACL::OID_GLOBALPLATFORM(ARRAY_AND_SIZE(oid_globalplatform));

	GPSEACL::GPSEACL():AccessControlList()
	{
	}

	GPSEACL::~GPSEACL()
	{
	}

	int GPSEACL::loadACL(Channel *channel)
	{
		int result = 0;
		PKCS15 *pkcs15;

		if (channel == NULL)
		{
			return -1;
		}

		pkcs15 = new PKCS15(channel, channel->getSelectResponse());
		if (pkcs15 != NULL)
		{
			/* basically, all requests will be accepted when PKCS #15 doesn't exist or global platform OID is not placed */
			allGranted = true;

			if (pkcs15->isClosed() == false)
			{
				PKCS15ODF *odf;

				if ((odf = pkcs15->getODF()) != NULL)
				{
					PKCS15DODF *dodf;

					if ((dodf = odf->getDODF()) != NULL)
					{
						if (loadAccessControl(channel, dodf) == 0)
						{
							printAccessControlList();

							result = 0;
						}
						else
						{
							SCARD_DEBUG_ERR("loadAccessControl failed, every request will be accepted.");
						}
					}
					else
					{
						SCARD_DEBUG_ERR("dodf null, every request will be accepted.");
					}
				}
				else
				{
					SCARD_DEBUG_ERR("odf null, every request will be accepted.");
				}
			}
			else
			{
				SCARD_DEBUG_ERR("failed to open PKCS15, every request will be accepted.");
			}

			delete pkcs15;
		}
		else
		{
			return -1;
		}

		return result;
	}

	int GPSEACL::loadAccessControl(Channel *channel, PKCS15DODF *dodf)
	{
		int result = -1;
		ByteArray path;

		if ((result = dodf->searchOID(OID_GLOBALPLATFORM, path)) == 0)
		{
			ByteArray data;
			FileObject file(channel);

			SCARD_DEBUG("oid path : %s", path.toString());

			file.select(NumberStream::getLittleEndianNumber(path));
			file.readBinary(0, 0, file.getFCP()->getFileSize(), data);

			SCARD_DEBUG("data : %s", data.toString());

			/* PKCS #15 and DODF OID exists. apply access control rule!! */
			allGranted = false;

			SimpleTLV tlv(data);

			if (tlv.decodeTLV() == true && tlv.getTag() == 0x30) /* SEQUENCE : AccessControlMainFile */
			{
				tlv.enterToValueTLV();

				/* refresh Tag */
				ByteArray refreshTag;

				refreshTag = SimpleTLV::getOctetString(tlv);
				SCARD_DEBUG("current refresh tag : %s", refreshTag.toString());

				if (this->refreshTag != refreshTag) /* need to update access control list */
				{
					this->refreshTag = refreshTag;

					releaseACL();

					/* access control rule path */
					if (tlv.decodeTLV() == true && tlv.getTag() == 0x30) /* SEQUENCE : Path */
					{
						/* TODO : parse path */
						ByteArray path;

						/* OCTET STRING */
						path = SimpleTLV::getOctetString(tlv.getValue());
						SCARD_DEBUG("access control rule path : %s", path.toString());

						if (loadRules(channel, path) == 0)
						{
							SCARD_DEBUG("loadRules success");
						}
						else
						{
							SCARD_DEBUG_ERR("loadRules failed");
						}
					}
				}
				tlv.returnToParentTLV();
			}
			else
			{
				SCARD_DEBUG_ERR("tlv.decodeTLV failed");
			}
		}
		else
		{
			SCARD_DEBUG_ERR("OID not found");
		}

		return result;
	}

	int GPSEACL::loadRules(Channel *channel, ByteArray path)
	{
		FileObject file(channel);
		ByteArray data, aid;

		file.select(NumberStream::getLittleEndianNumber(path));
		file.readBinary(0, 0, file.getFCP()->getFileSize(), data);

		SCARD_DEBUG("data : %s", data.toString());

		SimpleTLV tlv(data);

		while (tlv.decodeTLV() == true && tlv.getTag() == 0x30) /* SEQUENCE : Rule */
		{
			tlv.enterToValueTLV();
			if (tlv.decodeTLV() == true)
			{
				/* target */
				switch (tlv.getTag())
				{
				case 0xA0 : /* CHOICE 0 : EXPLICIT AID */
					/* OCTET STRING */
					aid = SimpleTLV::getOctetString(tlv.getValue());
					break;

				case 0x81 : /* CHOICE 1?? : default */
					aid = AccessControlList::AID_DEFAULT;
					break;

				case 0x82 : /* CHOICE 2?? : any application */
					aid = AccessControlList::AID_ALL;
					break;
				}

				SCARD_DEBUG("aid : %s", aid.toString());

				/* access condition path */
				if (tlv.decodeTLV() == true && tlv.getTag() == 0x30) /* SEQUENCE : Path */
				{
					ByteArray path;

					/* OCTET STRING */
					path = SimpleTLV::getOctetString(tlv.getValue());
					SCARD_DEBUG("path : %s", path.toString());

					if (loadAccessConditions(channel, aid, path) == 0)
					{
						SCARD_DEBUG("loadCertHashes success");
					}
					else
					{
						SCARD_DEBUG_ERR("loadCertHashes failed");
					}
				}
				else
				{
					SCARD_DEBUG_ERR("decodeTLV failed");
				}
			}
			else
			{
				SCARD_DEBUG_ERR("decodeTLV failed");
			}
			tlv.returnToParentTLV();
		}

		return 0;
	}

	int GPSEACL::loadAccessConditions(Channel *channel, ByteArray aid, ByteArray path)
	{
		FileObject file(channel);
		ByteArray data;

		file.select(NumberStream::getLittleEndianNumber(path));
		file.readBinary(0, 0, file.getFCP()->getFileSize(), data);

		SCARD_DEBUG("data : %s", data.toString());

		AccessCondition condition;

		condition.loadAccessCondition(aid, data);

		pair<ByteArray, AccessCondition> newItem(aid, condition);

		mapConditions.insert(newItem);

		return 0;
	}

} /* namespace smartcard_service_api */

/* export C API */
#define GP_SE_ACL_EXTERN_BEGIN \
	if (handle != NULL) \
	{ \
		GPSEACL *acl = (GPSEACL *)handle;

#define GP_SE_ACL_EXTERN_END \
	} \
	else \
	{ \
		SCARD_DEBUG_ERR("Invalid param"); \
	}

using namespace smartcard_service_api;

EXTERN_API gp_se_acl_h gp_se_acl_create_instance()
{
	GPSEACL *acl = new GPSEACL();

	return (gp_se_acl_h)acl;
}

EXTERN_API int gp_se_acl_load_acl(gp_se_acl_h handle, channel_h channel)
{
	int result = -1;

	GP_SE_ACL_EXTERN_BEGIN;
	result = acl->loadACL((Channel *)channel);
	GP_SE_ACL_EXTERN_END;

	return result;
}

EXTERN_API int gp_se_acl_update_acl(gp_se_acl_h handle, channel_h channel)
{
	int result = -1;

	GP_SE_ACL_EXTERN_BEGIN;
	acl->updateACL((Channel *)channel);
	GP_SE_ACL_EXTERN_END;

	return result;
}

EXTERN_API void gp_se_acl_release_acl(gp_se_acl_h handle)
{
	GP_SE_ACL_EXTERN_BEGIN;
	acl->releaseACL();
	GP_SE_ACL_EXTERN_END;
}

EXTERN_API bool gp_se_acl_is_authorized_access(gp_se_acl_h handle, unsigned char *aidBuffer, unsigned int aidLength, unsigned char *certHashBuffer, unsigned int certHashLength)
{
	bool result = false;

	GP_SE_ACL_EXTERN_BEGIN;
	result = acl->isAuthorizedAccess(aidBuffer, aidLength, certHashBuffer, certHashLength);
	GP_SE_ACL_EXTERN_END;

	return result;
}

EXTERN_API void gp_se_acl_destroy_instance(gp_se_acl_h handle)
{
	GP_SE_ACL_EXTERN_BEGIN;
	delete acl;
	GP_SE_ACL_EXTERN_END;
}
