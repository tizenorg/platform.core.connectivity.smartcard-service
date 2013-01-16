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

/* SLP library header */

/* local header */
#include "Debug.h"
#include "FCI.h"
#include "SimpleTLV.h"
//#include "ISO7816BERTLV.h"
#include "NumberStream.h"

namespace smartcard_service_api
{
	/* FCP class method */
	FCP::FCP()
	{
		resetMemberVar();
	}

	FCP::FCP(ByteArray &array)
	{
		resetMemberVar();

		setFCP(array);
	}

	FCP::~FCP()
	{
	}

	void FCP::resetMemberVar()
	{
		fileSize = FCI::INFO_NOT_AVAILABLE;
		totalFileSize = FCI::INFO_NOT_AVAILABLE;
		fid = FCI::INFO_NOT_AVAILABLE;
		sfi = FCI::INFO_NOT_AVAILABLE;
		maxRecordSize = FCI::INFO_NOT_AVAILABLE;
		numberOfRecord = FCI::INFO_NOT_AVAILABLE;
		fileType = FCI::INFO_NOT_AVAILABLE;
		fileStructure = FCI::INFO_NOT_AVAILABLE;
		lcs = FCI::INFO_NOT_AVAILABLE;
	}

	bool FCP::setFCP(ByteArray array)
	{
		bool result = false;
		SimpleTLV tlv;

		SCARD_BEGIN();

		releaseFCP();

		if (array.getLength() == 0)
			return false;

		fcpBuffer = array;

		if (fcpBuffer[0] != 0x62)
		{
			SCARD_DEBUG_ERR("it is not FCP response [%02X]", fcpBuffer[0]);
			return false;
		}

		/* parse... */
		tlv.setTLVBuffer(fcpBuffer.getBuffer(), fcpBuffer.getLength());

		if (tlv.decodeTLV())
		{
			tlv.enterToValueTLV();

			while (tlv.decodeTLV())
			{
				switch (tlv.getTag())
				{
				case 0x80 : /* file length without sturctural inforamtion */
					{
						SCARD_DEBUG("0x%02X : file length without sturctural inforamtion : %s", tlv.getTag(), tlv.getValue().toString());
						if (tlv.getLength() > 0)
						{
							fileSize = NumberStream::getBigEndianNumber(tlv.getValue());
						}
					}
					break;

				case 0x81 : /* file length with sturctural inforamtion */
					{
						SCARD_DEBUG("0x%02X : file length with sturctural inforamtion : %s", tlv.getTag(), tlv.getValue().toString());
						if (tlv.getLength() > 0)
						{
							maxRecordSize = NumberStream::getBigEndianNumber(tlv.getValue());
						}
					}
					break;

				case 0x82 : /* file descriptor bytes */
					{
						SCARD_DEBUG("0x%02X : file descriptor bytes : %s", tlv.getTag(), tlv.getValue().toString());
	//					ByteArray value = tlv.getValue();
					}
					break;

				case 0x83 : /* file identifier */
					{
						SCARD_DEBUG("0x%02X : file identifier : %s", tlv.getTag(), tlv.getValue().toString());
						if (tlv.getLength() > 0)
						{
							ByteArray value = tlv.getValue();

							fid = 0;

							memcpy(&fid, value.getBuffer(), value.getLength());
						}
					}
					break;

				case 0x84 : /* DF name */
					{
						SCARD_DEBUG("0x%02X : DF name : %s", tlv.getTag(), tlv.getValue().toString());
	//					ByteArray value = tlv.getValue();
					}
					break;

				case 0x85 : /* proprietary information not encoded in BER-TLV */
					{
						SCARD_DEBUG("0x%02X : proprietary information not encoded in BER-TLV : %s", tlv.getTag(), tlv.getValue().toString());
	//					ByteArray value = tlv.getValue();
					}
					break;

				case 0x86 : /* Security attribute in proprietary format */
					{
						SCARD_DEBUG("0x%02X : Security attribute in proprietary format : %s", tlv.getTag(), tlv.getValue().toString());
	//					ByteArray value = tlv.getValue();
					}
					break;

				case 0x87 : /* Identifier of an EF containing an extension of the file control information */
					{
						SCARD_DEBUG("0x%02X : Identifier of an EF containing an extension of the file control information : %s", tlv.getTag(), tlv.getValue().toString());
	//					ByteArray value = tlv.getValue();
					}
					break;

				case 0x88 : /* Short EF identifier */
					{
						SCARD_DEBUG("0x%02X : Short EF identifier : %s", tlv.getTag(), tlv.getValue().toString());

						if (tlv.getLength() > 0)
						{
							ByteArray value = tlv.getValue();

							sfi = 0;

							memcpy(&sfi, value.getBuffer(), value.getLength());
						}
					}
					break;

				case 0x8A : /* life cycle status byte */
					{
						SCARD_DEBUG("0x%02X : life cycle status byte : %s", tlv.getTag(), tlv.getValue().toString());
						if (tlv.getLength() > 0)
						{
							ByteArray value = tlv.getValue();

							lcs = 0;

							memcpy(&lcs, value.getBuffer(), value.getLength());
						}
					}
					break;

				case 0x8B : /* Security attribute referencing the expanded format */
					{
						SCARD_DEBUG("0x%02X : Security attribute referencing the expanded format : %s", tlv.getTag(), tlv.getValue().toString());
	//					ByteArray value = tlv.getValue();
					}
					break;

				case 0x8C : /* Security attribute in compact format */
					{
						SCARD_DEBUG("0x%02X : Security attribute in compact format : %s", tlv.getTag(), tlv.getValue().toString());
	//					ByteArray value = tlv.getValue();
					}
					break;

				case 0x8D : /* Identifier of an EF containing security environment templates */
					{
						SCARD_DEBUG("0x%02X : Identifier of an EF containing security environment templates : %s", tlv.getTag(), tlv.getValue().toString());
	//					ByteArray value = tlv.getValue();
					}
					break;

				case 0x8E : /* Channel security attribute */
					{
						SCARD_DEBUG("0x%02X : Channel security attribute : %s", tlv.getTag(), tlv.getValue().toString());
	//					ByteArray value = tlv.getValue();
					}
					break;

				case 0xA0 : /* Security attribute template for data objects */
					{
						SCARD_DEBUG("0x%02X : Security attribute template for data objects : %s", tlv.getTag(), tlv.getValue().toString());
	//					ByteArray value = tlv.getValue();
					}
					break;

				case 0xA1 : /* Security attribute template in proprietary format */
					{
						SCARD_DEBUG("0x%02X : Security attribute template in proprietary format : %s", tlv.getTag(), tlv.getValue().toString());
	//					ByteArray value = tlv.getValue();
					}
					break;

				case 0xA2 : /* Template consisting of one or more pairs of data objects */
					{
						SCARD_DEBUG("0x%02X : Template consisting of one or more pairs of data objects : %s", tlv.getTag(), tlv.getValue().toString());
	//					ByteArray value = tlv.getValue();
					}
					break;

				case 0xA5 : /* proprietary information encoded in BER-TLV */
					{
						SCARD_DEBUG("0x%02X : proprietary information encoded in BER-TLV : %s", tlv.getTag(), tlv.getValue().toString());
	//					ByteArray value = tlv.getValue();
					}
					break;

				case 0xAB : /* Security attribute template in expanded format */
					{
						SCARD_DEBUG("0x%02X : Security attribute template in expanded format : %s", tlv.getTag(), tlv.getValue().toString());
	//					ByteArray value = tlv.getValue();
					}
					break;

				case 0xAC : /* Cryptographic mechanism identifier template */
					{
						SCARD_DEBUG("0x%02X : Cryptographic mechanism identifier template : %s", tlv.getTag(), tlv.getValue().toString());
	//					ByteArray value = tlv.getValue();
					}
					break;

				case 0xC6 : /* PIN status template DO */
					{
						SCARD_DEBUG("0x%02X : PIN status template DO : %s", tlv.getTag(), tlv.getValue().toString());
	//					ByteArray value = tlv.getValue();
					}
					break;

				default :
					{
						SCARD_DEBUG("0x%02X : unknown : %s", tlv.getTag(), tlv.getValue().toString());
					}
					break;
				}
			}
			tlv.returnToParentTLV();
		}
		else
		{
			SCARD_DEBUG_ERR("tlv.decodeTLV failed");
		}

		SCARD_END();

		return result;
	}

	ByteArray FCP::getFCP()
	{
		return fcpBuffer;
	}

	void FCP::releaseFCP()
	{
		fcpBuffer.releaseBuffer();

		resetMemberVar();
	}

	unsigned int FCP::getFileSize()
	{
		return fileSize;
	}

	unsigned int FCP::getTotalFileSize()
	{
		return totalFileSize;
	}

	unsigned int FCP::getFID()
	{
		return fid;
	}

	unsigned int FCP::getSFI()
	{
		return sfi;
	}

	unsigned int FCP::getMaxRecordSize()
	{
		return maxRecordSize;
	}

	unsigned int FCP::getNumberOfRecord()
	{
		return numberOfRecord;
	}

	unsigned int FCP::getFileType()
	{
		return fileType;
	}

	unsigned int FCP::getFileStructure()
	{
		return fileStructure;
	}

	unsigned int FCP::getLCS()
	{
		return lcs;
	}

	const char *FCP::toString()
	{
		memset(strBuffer, 0, sizeof(strBuffer));

		snprintf(strBuffer, sizeof(strBuffer), "size [%d], total size [%d], fid [%x], sfi [%x], max rec [%d], n of rec [%d], type [%d], struct [%d], lcs [%d]",
			getFileSize(), getTotalFileSize(), getFID(), getSFI(), getMaxRecordSize(), getNumberOfRecord(), getFileType(), getFileStructure(), getLCS());

		return (const char *)strBuffer;
	}


	/* FCM class method */
	FCM::FCM()
	{
	}

	FCM::~FCM()
	{
	}


	/* FCI class method */
	FCI::FCI()
	{
	}

	FCI::~FCI()
	{
	}

	bool FCI::setFCIBuffer(ByteArray array)
	{
		bool result = false;

		return result;
	}

} /* namespace smartcard_service_api */
