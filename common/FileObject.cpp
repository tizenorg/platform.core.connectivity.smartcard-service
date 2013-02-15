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

/* SLP library header */

/* local header */
#include "Debug.h"
#include "FileObject.h"
#include "APDUHelper.h"

namespace smartcard_service_api
{
	FileObject::FileObject(Channel *channel):ProviderHelper(channel)
	{
		opened = false;
	}

	FileObject::FileObject(Channel *channel, ByteArray selectResponse):ProviderHelper(channel)
	{
		opened = false;
		setSelectResponse(selectResponse);
	}

	FileObject::~FileObject()
	{
	}

	bool FileObject::setSelectResponse(ByteArray &response)
	{
		bool result = false;

		if (response.getLength() >= 2)
		{
			ResponseHelper resp(response);
			selectResponse = response;

			if (resp.getStatus() == 0)
			{
				fcp.releaseFCP();

				fcp.setFCP(resp.getDataField());

				SCARD_DEBUG("FCP : %s", fcp.toString());

				opened = true;
				result = true;
			}
			else
			{
				SCARD_DEBUG_ERR("status word [%d][ %02X %02X ]", resp.getStatus(), resp.getSW1(), resp.getSW2());
			}
		}
		else
		{
			SCARD_DEBUG_ERR("invalid response : %s", response.toString());
		}

		return result;
	}

	int FileObject::_select(ByteArray command)
	{
		int ret = ERROR_ILLEGAL_STATE;
		ByteArray result;

		if (channel == NULL || channel->isClosed())
		{
			SCARD_DEBUG_ERR("channel is not open");

			return ret;
		}

		opened = false;

		ret = channel->transmitSync(command, result);
		if (ret == 0)
		{
			if (setSelectResponse(result) == true)
			{
				ret = SUCCESS;
			}
			else
			{
				ret = ERROR_ILLEGAL_STATE;
			}
		}
		else
		{
			SCARD_DEBUG_ERR("select apdu is failed, rv [%d], length [%d]", ret, result.getLength());

			ret = ERROR_ILLEGAL_STATE;
		}

		return ret;
	}

	int FileObject::select(ByteArray aid)
	{
		int ret = ERROR_ILLEGAL_STATE;
		ByteArray command;

		/* make apdu command */
		command = APDUHelper::generateAPDU(APDUHelper::COMMAND_SELECT_BY_DF_NAME, 0, aid);
		SCARD_DEBUG("command : %s", command.toString());

		ret = _select(command);

		return ret;
	}

	int FileObject::select(ByteArray path, bool fromCurrentDF)
	{
		int ret = ERROR_ILLEGAL_STATE;
		ByteArray command;

		/* make apdu command */
		if (fromCurrentDF == true)
		{
			command = APDUHelper::generateAPDU(APDUHelper::COMMAND_SELECT_BY_PATH_FROM_CURRENT_DF, 0, path);
		}
		else
		{
			command = APDUHelper::generateAPDU(APDUHelper::COMMAND_SELECT_BY_PATH, 0, path);
		}
		SCARD_DEBUG("command : %s", command.toString());

		ret = _select(command);

		return ret;
	}

	int FileObject::select(unsigned int fid)
	{
		int ret = ERROR_ILLEGAL_STATE;
		ByteArray command, fidData((unsigned char *)&fid, 2);

		/* make apdu command */
		command = APDUHelper::generateAPDU(APDUHelper::COMMAND_SELECT_BY_ID, 0, fidData);
		SCARD_DEBUG("command : %s", command.toString());

		ret = _select(command);

		return ret;
	}

	int FileObject::selectParent()
	{
		int ret = ERROR_ILLEGAL_STATE;
		ByteArray command;

		/* make apdu command */
		command = APDUHelper::generateAPDU(APDUHelper::COMMAND_SELECT_PARENT_DF, 0, ByteArray::EMPTY);
		SCARD_DEBUG("command : %s", command.toString());

		ret = _select(command);

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
				SCARD_DEBUG_ERR("status word [%d][ %02X %02X ]", resp.getStatus(), resp.getSW1(), resp.getSW2());
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
				SCARD_DEBUG_ERR("status word [%d][ %02X %02X ]", resp.getStatus(), resp.getSW1(), resp.getSW2());
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
				SCARD_DEBUG_ERR("status word [%d][ %02X %02X ]", resp.getStatus(), resp.getSW1(), resp.getSW2());
			}
		}
		else
		{
			SCARD_DEBUG_ERR("select apdu is failed, rv [%d], length [%d]", ret, response.getLength());
		}

		return ret;
	}

} /* namespace smartcard_service_api */
