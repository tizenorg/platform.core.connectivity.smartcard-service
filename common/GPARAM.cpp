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
#include "GPARAM.h"
#include "APDUHelper.h"
#include "FileObject.h"
#include "NumberStream.h"
#include "SimpleTLV.h"
#include "ISO7816BERTLV.h"
#include "AccessCondition.h"

#ifndef EXTERN_API
#define EXTERN_API __attribute__((visibility("default")))
#endif

namespace smartcard_service_api
{
	static unsigned char aid_aram[] = { 0xA0, 0x00, 0x00, 0x01, 0x51, 0x41, 0x43, 0x4C, 00 };
	static ByteArray AID_ARAM(ARRAY_AND_SIZE(aid_aram));

#define GET_DATA_ALL		0
#define GET_DATA_SPECIFIC	1
#define GET_DATA_REFRESH_TAG	2
#define GET_DATA_NEXT		3

#define ARAM_TAG_ALL_AR		0x0000FF40
#define ARAM_TAG_AR		0x0000FF50
#define ARAM_TAG_REFRESH	0x0000DF20

#define DO_TAG_AID_REF		0x0000004F
#define DO_TAG_AID_REF_DEFAULT	0x000000C0
#define DO_TAG_HASH_REF		0x000000C1
#define DO_TAG_APDU_AR		0x000000D0
#define DO_TAG_NFC_AR		0x000000D1
#define DO_TAG_REF		0x000000E1
#define DO_TAG_REF_AR		0x000000E2
#define DO_TAG_AR		0x000000E3

	GPARAM::GPARAM(Channel *channel)
		: FileObject(channel)
	{
	}

	int GPARAM::select()
	{
		return FileObject::select(AID_ARAM);
	}

	static int doTransmit(Channel *channel, const ByteArray &command, ByteArray &response)
	{
		int result;
		ByteArray resp;

		_BEGIN();

		result = channel->transmitSync(command, resp);
		if (result == SCARD_ERROR_OK) {
			result = ResponseHelper::getStatus(resp);
			if (result >= SCARD_ERROR_OK) {
				response = ResponseHelper::getDataField(resp);
				_DBG("response[%d] : %s", response.size(), response.toString().c_str());
			} else {
				_ERR("transmit returns error, [%d]", result);
			}
		} else {
			_ERR("transmitSync failed, [%d]", result);
		}

		_END();

		return result;
	}

	static int doCommand(Channel *channel, int command, ByteArray &response)
	{
		int result;
		APDUCommand helper;
		ByteArray cmd, resp;

		_BEGIN();

		switch (command) {
		case GET_DATA_ALL :
			helper.setCommand(0x80, 0xCA, 0xFF, 0x40, ByteArray::EMPTY, 0);
			break;

		case GET_DATA_REFRESH_TAG :
			helper.setCommand(0x80, 0xCA, 0xDF, 0x20, ByteArray::EMPTY, 0);
			break;

		case GET_DATA_NEXT :
			helper.setCommand(0x80, 0xCA, 0xFF, 0x60, ByteArray::EMPTY, 0);
			break;
		}

		helper.getBuffer(cmd);

		_DBG("command[%d] : %s", cmd.size(), cmd.toString().c_str());

		result = doTransmit(channel, cmd, response);

		_END();

		return result;
	}

	static int doCommand(Channel *channel, ByteArray &data, ByteArray &response)
	{
		int result;
		APDUCommand helper;
		ByteArray cmd;

		helper.setCommand(0x80, 0xCA, 0xFF, 0x50, data, 0);
		helper.getBuffer(cmd);

		result = doTransmit(channel, cmd, response);

		return result;
	}

	int GPARAM::getDataAll(ByteArray &data)
	{
		int result;
		ByteArray response;

		_BEGIN();

		result = doCommand(channel, GET_DATA_ALL, response);
		if (result >= SCARD_ERROR_OK) {
			ISO7816BERTLV tlv(response);

			if (tlv.decodeTLV() == true &&
				tlv.getTag() == ARAM_TAG_ALL_AR) {
				unsigned int length = tlv.size();

				if (length > 0){
					data = tlv.getValue();

					while (length > data.size()) {
						result = doCommand(channel, GET_DATA_NEXT, response);
						if (result >= SCARD_ERROR_OK) {
							data += response;
						} else {
							_ERR("generateCommand failed, [%d]", result);
							data.clear();
							break;
						}
					}

					_DBG("data[%d] : %s", data.size(), data.toString().c_str());
				} else {
					_INFO("Response-ALL-AR-DO is empty");
					data.clear();
				}
			} else {
				_ERR("decodeTLV failed, %s", response.toString().c_str());
				result = SCARD_ERROR_ILLEGAL_PARAM;
			}
		} else {
			_ERR("generateCommand failed, [%d]", result);
		}

		_END();

		return result;
	}

	static int createRefDo(const ByteArray &aid, const ByteArray &hash, ByteArray &refDo)
	{
		ByteArray temp;

		temp = SimpleTLV::encode(DO_TAG_AID_REF, aid);
		temp += SimpleTLV::encode(DO_TAG_HASH_REF, hash);

		refDo = SimpleTLV::encode(DO_TAG_REF, temp);
		_DBG("encoded Ref DO : %s", refDo.toString().c_str());

		return SCARD_ERROR_OK;
	}

	int GPARAM::getDataSpecific(const ByteArray &aid, const ByteArray &hash,
		ByteArray &data)
	{
		int result;
		ByteArray refDo, response;

		_BEGIN();

		createRefDo(aid, hash, refDo);

		result = doCommand(channel, refDo, response);
		if (result >= SCARD_ERROR_OK) {
			ISO7816BERTLV tlv(response);

			if (tlv.decodeTLV() == true &&
				tlv.getTag() == ARAM_TAG_AR) {
				unsigned int length = tlv.size();

				if (length > 0){
					data = tlv.getValue();

					while (length > data.size()) {
						result = doCommand(channel, GET_DATA_NEXT, response);
						if (result >= SCARD_ERROR_OK) {
							data += response;
						} else {
							_ERR("generateCommand failed, [%d]", result);
							data.clear();
							break;
						}
					}
					_DBG("data[%d] : %s", data.size(), data.toString().c_str());
				} else {
					_INFO("Response-ALL-AR-DO is empty");
					data.clear();
				}
			} else {
				_ERR("decodeTLV failed, %s", response.toString().c_str());
				result = SCARD_ERROR_ILLEGAL_PARAM;
			}
		} else {
			_ERR("doCommand failed, [%d]", result);
		}

		_END();

		return result;
	}

	int GPARAM::getDataRefreshTag(ByteArray &tag)
	{
		int result;
		ByteArray response;

		_BEGIN();

		result = doCommand(channel, GET_DATA_REFRESH_TAG, response);
		if (result >= SCARD_ERROR_OK) {
			ISO7816BERTLV tlv(response);

			if (tlv.decodeTLV() == true &&
				tlv.getTag() == ARAM_TAG_REFRESH &&
				tlv.size() == 8) {
				tag = tlv.getValue();
				result = SCARD_ERROR_OK;
				_DBG("refreshTag[%d] : %s", tag.size(), tag.toString().c_str());
			} else {
				_ERR("decodeTLV failed, %s", response.toString().c_str());
				result = SCARD_ERROR_ILLEGAL_PARAM;
			}
		} else {
			_ERR("generateCommand failed, [%d]", result);
		}

		_END();

		return result;
	}
} /* namespace smartcard_service_api */
