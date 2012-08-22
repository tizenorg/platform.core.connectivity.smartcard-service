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

/* SLP library header */

/* local header */
#include "Debug.h"
#include "FileObject.h"
#include "APDUHelper.h"

namespace smartcard_service_api
{
	FileObject::FileObject(Channel *channel):ProviderHelper(channel)
	{
	}

	FileObject::FileObject(Channel *channel, ByteArray selectResponse):ProviderHelper(channel)
	{
		setSelectResponse(selectResponse);
	}

	FileObject::~FileObject()
	{
	}

	bool FileObject::setSelectResponse(ByteArray response)
	{
		bool result = false;

		selectResponse = response;

		if (selectResponse.getLength() > 2)
		{
			ResponseHelper resp(selectResponse);

			fcp.setFCP(resp.getDataField());

			result = true;
		}
		else
		{
			SCARD_DEBUG_ERR("invalid response : %s", selectResponse.toString());
		}

		return result;
	}

	int FileObject::select(ByteArray aid)
	{
		int ret = ERROR_ILLEGAL_STATE;
		ByteArray command, result;
		APDUHelper apdu;

		if (channel == NULL || channel->isClosed())
		{
			SCARD_DEBUG_ERR("channel is not open");

			return ret;
		}

		/* make apdu command */
		command = apdu.generateAPDU(APDUHelper::COMMAND_SELECT_BY_DF_NAME, 0, aid);
		SCARD_DEBUG("command : %s", command.toString());
		ret = channel->transmitSync(command, result);

		if (ret == 0 && result.getLength() >= 2)
		{
			ResponseHelper resp(result);
			this->selectResponse = result;

			if (resp.getStatus() == 0)
			{
				SCARD_DEBUG("response [%d] : %s", result.getLength(), result.toString());

				fcp.releaseFCP();

				if (result.getLength() > 2)
				{
					fcp.setFCP(resp.getDataField());

					SCARD_DEBUG("FCP : %s", fcp.toString());
				}

				ret = SUCCESS;
			}
			else
			{
				SCARD_DEBUG_ERR("status word [%d][ 0x%02X 0x%02X ]", resp.getStatus(), result[result.getLength() - 2], result[result.getLength() - 1]);
			}
		}
		else
		{
			SCARD_DEBUG_ERR("select apdu is failed, rv [%d], length [%d]", ret, result.getLength());
		}

		return ret;
	}

	int FileObject::select(ByteArray path, bool fromCurrentDF)
	{
		int ret = ERROR_ILLEGAL_STATE;
		ByteArray command, result;
		APDUHelper apdu;

		if (channel == NULL || channel->isClosed())
		{
			SCARD_DEBUG_ERR("channel is not open");

			return ret;
		}

		/* make apdu command */
		if (fromCurrentDF == true)
		{
			command = apdu.generateAPDU(APDUHelper::COMMAND_SELECT_BY_PATH_FROM_CURRENT_DF, 0, path);
		}
		else
		{
			command = apdu.generateAPDU(APDUHelper::COMMAND_SELECT_BY_PATH, 0, path);
		}
		SCARD_DEBUG("command : %s", command.toString());

		ret = channel->transmitSync(command, result);

		if (ret == 0 && result.getLength() >= 2)
		{
			ResponseHelper resp(result);
			this->selectResponse = result;

			if (resp.getStatus() == 0)
			{
				SCARD_DEBUG("response [%d] : %s", result.getLength(), result.toString());

				fcp.releaseFCP();

				if (result.getLength() > 2)
				{
					fcp.setFCP(resp.getDataField());

					SCARD_DEBUG("FCP : %s", fcp.toString());
				}

				ret = SUCCESS;
			}
			else
			{
				SCARD_DEBUG_ERR("status word [%d][ 0x%02X 0x%02X ]", resp.getStatus(), result[result.getLength() - 2], result[result.getLength() - 1]);
			}
		}
		else
		{
			SCARD_DEBUG_ERR("select apdu is failed, rv [%d], length [%d]", ret, result.getLength());
		}

		return ret;
	}

	int FileObject::select(unsigned int fid)
	{
		int ret = ERROR_ILLEGAL_STATE;
		ByteArray command, result, fidData((unsigned char *)&fid, 2);

		if (channel == NULL || channel->isClosed())
		{
			SCARD_DEBUG_ERR("channel is not open");

			return ret;
		}

		/* make apdu command */
		command = APDUHelper::generateAPDU(APDUHelper::COMMAND_SELECT_BY_ID, 0, fidData);
		ret = channel->transmitSync(command, result);

		if (ret == 0 && result.getLength() >= 2)
		{
			ResponseHelper resp(result);
			this->selectResponse = result;

			if (resp.getStatus() == 0)
			{
				SCARD_DEBUG("response [%d] : %s", result.getLength(), result.toString());

				fcp.releaseFCP();

				if (result.getLength() > 2)
				{
					fcp.setFCP(resp.getDataField());

					SCARD_DEBUG("FCP : %s", fcp.toString());
				}

				ret = SUCCESS;
			}
			else
			{
				SCARD_DEBUG_ERR("status word [%d][ 0x%02X 0x%02X ]", resp.getStatus(), result[result.getLength() - 2], result[result.getLength() - 1]);
			}
		}
		else
		{
			SCARD_DEBUG_ERR("select apdu is failed, rv [%d], length [%d]", ret, result.getLength());
		}

		return ret;
	}

	int FileObject::selectParent()
	{
		int ret = ERROR_ILLEGAL_STATE;
		ByteArray command, result;
		APDUHelper apdu;

		if (channel == NULL || channel->isClosed())
		{
			SCARD_DEBUG_ERR("channel is not open");

			return ret;
		}

		/* make apdu command */
		command = apdu.generateAPDU(APDUHelper::COMMAND_SELECT_PARENT_DF, 0, ByteArray::EMPTY);
		SCARD_DEBUG("command : %s", command.toString());

		ret = channel->transmitSync(command, result);

		if (ret == 0 && result.getLength() >= 2)
		{
			ResponseHelper resp(result);
			this->selectResponse = result;

			if (resp.getStatus() == 0)
			{
				SCARD_DEBUG("response [%d] : %s", result.getLength(), result.toString());

				fcp.releaseFCP();

				if (result.getLength() > 2)
				{
					fcp.setFCP(resp.getDataField());

					SCARD_DEBUG("FCP : %s", fcp.toString());
				}

				ret = SUCCESS;
			}
			else
			{
				SCARD_DEBUG_ERR("status word [%d][ 0x%02X 0x%02X ]", resp.getStatus(), result[result.getLength() - 2], result[result.getLength() - 1]);
			}
		}
		else
		{
			SCARD_DEBUG_ERR("select apdu is failed, rv [%d], length [%d]", ret, result.getLength());
		}

		return ret;
	}

	FCI *FileObject::getFCI()
	{
		return NULL;
	}

	FCP *FileObject::getFCP()
	{
		return &fcp;
	}

	int FileObject::readRecord(unsigned int sfi, unsigned int recordId, Record &result)
	{
		ByteArray command, response;
		APDUCommand apdu;
		int ret;

		apdu.setCommand(0, APDUCommand::INS_READ_RECORD, recordId, 4, ByteArray::EMPTY, 0);
		apdu.getBuffer(command);
		SCARD_DEBUG("command : %s", command.toString());

		ret = channel->transmitSync(command, response);
		if (ret == 0 && response.getLength() >= 2)
		{
			ResponseHelper resp(response);

			if (resp.getStatus() == 0)
			{
				SCARD_DEBUG("response [%d] : %s", response.getLength(), response.toString());

//				result = resp.getDataField();

				ret = SUCCESS;
			}
			else
			{
				SCARD_DEBUG_ERR("status word [%d][ 0x%02X 0x%02X ]", resp.getStatus(), response[response.getLength() - 2], response[response.getLength() - 1]);
			}
		}
		else
		{
			SCARD_DEBUG_ERR("select apdu is failed, rv [%d], length [%d]", ret, response.getLength());
		}

		return ret;
	}

	int FileObject::writeRecord(unsigned int sfi, Record record)
	{
		return 0;
	}

	int FileObject::searchRecord(unsigned int sfi, ByteArray searchParam, vector<int> &result)
	{
		return 0;
	}

	int FileObject::readBinary(unsigned int sfi, unsigned int offset, unsigned int length, ByteArray &result)
	{
		ByteArray command, response;
		APDUCommand apdu;
		int ret;

		apdu.setCommand(0, APDUCommand::INS_READ_BINARY, offset, 0, ByteArray::EMPTY, length);
		apdu.getBuffer(command);
		SCARD_DEBUG("command : %s", command.toString());

		ret = channel->transmitSync(command, response);
		if (ret == 0 && response.getLength() >= 2)
		{
			ResponseHelper resp(response);

			if (resp.getStatus() == 0)
			{
				SCARD_DEBUG("response [%d] : %s", response.getLength(), response.toString());

				result = resp.getDataField();

				ret = SUCCESS;
			}
			else
			{
				SCARD_DEBUG_ERR("status word [%d][ 0x%02X 0x%02X ]", resp.getStatus(), response[response.getLength() - 2], response[response.getLength() - 1]);
			}
		}
		else
		{
			SCARD_DEBUG_ERR("select apdu is failed, rv [%d], length [%d]", ret, response.getLength());
		}

		return ret;
	}

	int FileObject::writeBinary(unsigned int sfi, ByteArray data, unsigned int offset, unsigned int length)
	{
		ByteArray command, response;
		APDUCommand apdu;
		int ret;

		apdu.setCommand(0, APDUCommand::INS_WRITE_BINARY, offset, 0, data, 0);
		apdu.getBuffer(command);
		SCARD_DEBUG("command : %s", command.toString());

		ret = channel->transmitSync(command, response);
		if (ret == 0 && response.getLength() >= 2)
		{
			ResponseHelper resp(response);

			if (resp.getStatus() == 0)
			{
				SCARD_DEBUG("response [%d] : %s", response.getLength(), response.toString());

				ret = SUCCESS;
			}
			else
			{
				SCARD_DEBUG_ERR("status word [%d][ 0x%02X 0x%02X ]", resp.getStatus(), response[response.getLength() - 2], response[response.getLength() - 1]);
			}
		}
		else
		{
			SCARD_DEBUG_ERR("select apdu is failed, rv [%d], length [%d]", ret, response.getLength());
		}

		return ret;
	}

} /* namespace smartcard_service_api */
