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

#include <cstdio>
#include <cstring>
#include <sstream>

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

	FCP::FCP(const ByteArray &array)
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

	bool FCP::setFCP(const ByteArray &array)
	{
		bool result = false;
		SimpleTLV tlv;

		_BEGIN();

		releaseFCP();

		if (array.size() == 0)
			return false;

		fcpBuffer = array;

		if (fcpBuffer[0] != 0x62)
		{
			_ERR("it is not FCP response [%02X]", fcpBuffer[0]);
			return false;
		}

		/* parse... */
		tlv.setTLVBuffer(fcpBuffer.getBuffer(), fcpBuffer.size());

		if (tlv.decodeTLV())
		{
			tlv.enterToValueTLV();

			while (tlv.decodeTLV())
			{
				switch (tlv.getTag())
				{
				case 0x80 : /* file length without structural information */
					{
						_DBG("0x%02X : file length without structural information : %s", tlv.getTag(), tlv.getValue().toString().c_str());
						if (tlv.size() > 0)
						{
							fileSize = NumberStream::getBigEndianNumber(tlv.getValue());
						}
					}
					break;

				case 0x81 : /* file length with structural information */
					{
						_DBG("0x%02X : file length with structural information : %s", tlv.getTag(), tlv.getValue().toString().c_str());
						if (tlv.size() > 0)
						{
							maxRecordSize = NumberStream::getBigEndianNumber(tlv.getValue());
						}
					}
					break;

				case 0x82 : /* file descriptor bytes */
					{
						_DBG("0x%02X : file descriptor bytes : %s", tlv.getTag(), tlv.getValue().toString().c_str());
	//					ByteArray value = tlv.getValue();
					}
					break;

				case 0x83 : /* file identifier */
					{
						_DBG("0x%02X : file identifier : %s", tlv.getTag(), tlv.getValue().toString().c_str());
						if (tlv.size() > 0)
						{
							ByteArray value = tlv.getValue();

							fid = 0;

							memcpy(&fid, value.getBuffer(), value.size());
						}
					}
					break;

				case 0x84 : /* DF name */
					{
	//					ByteArray value = tlv.getValue();
					}
					break;

				case 0x85 : /* proprietary information not encoded in BER-TLV */
					{
						_DBG("0x%02X : proprietary information not encoded in BER-TLV : %s", tlv.getTag(), tlv.getValue().toString().c_str());
	//					ByteArray value = tlv.getValue();
					}
					break;

				case 0x86 : /* Security attribute in proprietary format */
					{
	//					ByteArray value = tlv.getValue();
					}
					break;

				case 0x87 : /* Identifier of an EF containing an extension of the file control information */
					{
						_DBG("0x%02X : Identifier of an EF containing an extension of the file control information : %s", tlv.getTag(), tlv.getValue().toString().c_str());
	//					ByteArray value = tlv.getValue();
					}
					break;

				case 0x88 : /* Short EF identifier */
					{
						_DBG("0x%02X : Short EF identifier : %s", tlv.getTag(), tlv.getValue().toString().c_str());

						if (tlv.size() > 0)
						{
							ByteArray value = tlv.getValue();

							sfi = 0;

							memcpy(&sfi, value.getBuffer(), value.size());
						}
					}
					break;

				case 0x8A : /* life cycle status byte */
					{
						_DBG("0x%02X : life cycle status byte : %s", tlv.getTag(), tlv.getValue().toString().c_str());
						if (tlv.size() > 0)
						{
							ByteArray value = tlv.getValue();

							lcs = 0;

							memcpy(&lcs, value.getBuffer(), value.size());
						}
					}
					break;

				case 0x8B : /* Security attribute referencing the expanded format */
					{
	//					ByteArray value = tlv.getValue();
					}
					break;

				case 0x8C : /* Security attribute in compact format */
					{
	//					ByteArray value = tlv.getValue();
					}
					break;

				case 0x8D : /* Identifier of an EF containing security environment templates */
					{
	//					ByteArray value = tlv.getValue();
					}
					break;

				case 0x8E : /* Channel security attribute */
					{
	//					ByteArray value = tlv.getValue();
					}
					break;

				case 0xA0 : /* Security attribute template for data objects */
					{
	//					ByteArray value = tlv.getValue();
					}
					break;

				case 0xA1 : /* Security attribute template in proprietary format */
					{
	//					ByteArray value = tlv.getValue();
					}
					break;

				case 0xA2 : /* Template consisting of one or more pairs of data objects */
					{
						_DBG("0x%02X : Template consisting of one or more pairs of data objects : %s", tlv.getTag(), tlv.getValue().toString().c_str());
	//					ByteArray value = tlv.getValue();
					}
					break;

				case 0xA5 : /* proprietary information encoded in BER-TLV */
					{
						_DBG("0x%02X : proprietary information encoded in BER-TLV : %s", tlv.getTag(), tlv.getValue().toString().c_str());
	//					ByteArray value = tlv.getValue();
					}
					break;

				case 0xAB : /* Security attribute template in expanded format */
					{
	//					ByteArray value = tlv.getValue();
					}
					break;

				case 0xAC : /* Cryptographic mechanism identifier template */
					{
						_DBG("0x%02X : Cryptographic mechanism identifier template : %s", tlv.getTag(), tlv.getValue().toString().c_str());
	//					ByteArray value = tlv.getValue();
					}
					break;

				case 0xC6 : /* PIN status template DO */
					{
	//					ByteArray value = tlv.getValue();
					}
					break;

				default :
					{
						_DBG("0x%02X : unknown : %s", tlv.getTag(), tlv.getValue().toString().c_str());
					}
					break;
				}
			}
			tlv.returnToParentTLV();
		}
		else
		{
			_ERR("tlv.decodeTLV failed");
		}

		_END();

		return result;
	}

	const ByteArray FCP::getFCP() const
	{
		return fcpBuffer;
	}

	void FCP::releaseFCP()
	{
		fcpBuffer.clear();

		resetMemberVar();
	}

	int FCP::getFileSize() const
	{
		return fileSize;
	}

	int FCP::getTotalFileSize() const
	{
		return totalFileSize;
	}

	int FCP::getFID() const
	{
		return fid;
	}

	int FCP::getSFI() const
	{
		return sfi;
	}

	int FCP::getMaxRecordSize() const
	{
		return maxRecordSize;
	}

	int FCP::getNumberOfRecord() const
	{
		return numberOfRecord;
	}

	int FCP::getFileType() const
	{
		return fileType;
	}

	int FCP::getFileStructure() const
	{
		return fileStructure;
	}

	int FCP::getLCS() const
	{
		return lcs;
	}

	const string FCP::toString() const
	{
		stringstream ss;

		if (fileSize != FCI::INFO_NOT_AVAILABLE)
			ss << "size [" << fileSize << "], ";

		if (totalFileSize != FCI::INFO_NOT_AVAILABLE)
			ss << "total size [" << totalFileSize << "], ";

		if (fid != FCI::INFO_NOT_AVAILABLE)
			ss << "fid [" << fid << "], ";

		if (sfi != FCI::INFO_NOT_AVAILABLE)
			ss << "sfi [" << sfi << "], ";

		if (maxRecordSize != FCI::INFO_NOT_AVAILABLE)
			ss << "max rec. [" << maxRecordSize << "], ";

		if (numberOfRecord != FCI::INFO_NOT_AVAILABLE)
			ss << "n of rec [" << numberOfRecord << "], ";

		if (fileType != FCI::INFO_NOT_AVAILABLE)
			ss << "type [" << fileType << "], ";

		if (fileStructure != FCI::INFO_NOT_AVAILABLE)
			ss << "struct [" << fileStructure << "], ";

		if (lcs != FCI::INFO_NOT_AVAILABLE)
			ss << "lcs [" << lcs << "], ";

		return ss.str();
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

	bool FCI::setFCIBuffer(const ByteArray &array)
	{
		bool result = false;

		return result;
	}

} /* namespace smartcard_service_api */
