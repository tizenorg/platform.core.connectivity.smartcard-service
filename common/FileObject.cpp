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
#include <glib.h>

/* SLP library header */

/* local header */
#include "Debug.h"
#include "FileObject.h"
#include "APDUHelper.h"

namespace smartcard_service_api
{
	FileObject::FileObject(Channel *channel)
		: ProviderHelper(channel)
	{
		opened = false;
	}

	FileObject::FileObject(Channel *channel, const ByteArray &selectResponse)
		: ProviderHelper(channel)
	{
		opened = false;
		setSelectResponse(selectResponse);
	}

	FileObject::~FileObject()
	{
		close();
	}

	void FileObject::close()
	{
		opened = false;
		selectResponse.clear();
	}

	bool FileObject::setSelectResponse(const ByteArray &response)
	{
		bool result = false;

		if (response.size() >= 2)
		{
			ResponseHelper resp(response);
			selectResponse = response;

			if (resp.getStatus() >= 0)
			{
				fcp.releaseFCP();

				fcp.setFCP(resp.getDataField());

				_DBG("FCP : %s", fcp.toString().c_str());

				opened = true;
				result = true;
			}
			else
			{
				_ERR("status word [ %02X %02X ]",
					resp.getSW1(), resp.getSW2());
			}
		}
		else
		{
			_ERR("invalid response");
		}

		return result;
	}

	int FileObject::_select(const ByteArray &command)
	{
		int ret = ERROR_ILLEGAL_STATE;
		ByteArray result;

		if (channel == NULL || channel->isClosed())
		{
			_ERR("channel is not open");

			return ret;
		}

		close();

		ret = channel->transmitSync(command, result);
		if (ret == 0)
		{
			ResponseHelper resp(result);

			ret = resp.getStatus();

			setSelectResponse(result);
		}
		else
		{
			_ERR("select apdu is failed, rv [%d], length [%d]",
				ret, result.size());

			ret = ERROR_ILLEGAL_STATE;
		}

		return ret;
	}

	int FileObject::select(const ByteArray &aid)
	{
		int ret = ERROR_ILLEGAL_STATE;
		ByteArray command;

		/* make apdu command */
		command = APDUHelper::generateAPDU(APDUHelper::COMMAND_SELECT_BY_DF_NAME, 0, aid);

		ret = _select(command);

		return ret;
	}

	int FileObject::select(const ByteArray &path, bool fromCurrentDF)
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
			ByteArray temp(path);

			if (path.size() > 2 && path[0] == 0x3f && path[1] == 0x00) /* check MF */
			{
				/* remove MF from path */
				temp.assign(path.getBuffer(2), path.size() - 2);
			}

			command = APDUHelper::generateAPDU(APDUHelper::COMMAND_SELECT_BY_PATH, 0, temp);
		}

		ret = _select(command);

		return ret;
	}

	int FileObject::select(unsigned int fid)
	{
		int ret = ERROR_ILLEGAL_STATE;
		ByteArray command, fidData((unsigned char *)&fid, 2);

		/* make apdu command */
		command = APDUHelper::generateAPDU(APDUHelper::COMMAND_SELECT_BY_ID, 0, fidData);

		ret = _select(command);

		return ret;
	}

	int FileObject::selectParent()
	{
		int ret = ERROR_ILLEGAL_STATE;
		ByteArray command;

		/* make apdu command */
		command = APDUHelper::generateAPDU(APDUHelper::COMMAND_SELECT_PARENT_DF, 0, ByteArray::EMPTY);

		ret = _select(command);

		return ret;
	}

	const FCI *FileObject::getFCI() const
	{
		return NULL;
	}

	const FCP *FileObject::getFCP() const
	{
		return &fcp;
	}

	int FileObject::readRecord(unsigned int sfi, unsigned int recordId, Record &result)
	{
		ByteArray command, response;
		APDUCommand apdu;
		int ret;

		apdu.setCommand(0, APDUCommand::INS_READ_RECORD, recordId, 4, ByteArray::EMPTY, APDUCommand::LE_MAX);
		apdu.getBuffer(command);

		ret = channel->transmitSync(command, response);
		if (ret == 0 && response.size() >= 2)
		{
			ResponseHelper resp(response);

			ret = resp.getStatus();
			if (ret >= 0)
			{
				_DBG("response [%d] : %s", response.size(), response.toString().c_str());

				result = Record(recordId, resp.getDataField());
			}
			else
			{
				_ERR("status word [ %02X %02X ]", resp.getSW1(), resp.getSW2());
			}
		}
		else
		{
			_ERR("select apdu is failed, rv [%d], length [%d]", ret, response.size());
		}

		return ret;
	}

	int FileObject::writeRecord(unsigned int sfi, const Record &record)
	{
		return 0;
	}

	int FileObject::searchRecord(unsigned int sfi, const ByteArray &searchParam, vector<int> &result)
	{
		return 0;
	}

#define MAX_SINGLE_LEN	256

	int FileObject::readBinary(unsigned int sfi, unsigned int offset, unsigned int length, ByteArray &result)
	{
		ByteArray command, response;
		APDUCommand apdu;
		int ret;

		/* FIXME : fix calculating length */
		apdu.setCommand(0, APDUCommand::INS_READ_BINARY,
			(offset >> 8) & 0x7F, offset & 0x00FF,
			ByteArray::EMPTY, (length > MAX_SINGLE_LEN - 1) ? 0 : length);

		apdu.getBuffer(command);

		ret = channel->transmitSync(command, response);
		if (ret == 0 && response.size() >= 2)
		{
			ResponseHelper resp(response);

			if (resp.getStatus() >= 0)
			{
				_DBG("response [%d] : %s", response.size(), response.toString().c_str());

				result = resp.getDataField();

				ret = SUCCESS;
			}
			else
			{
				_ERR("status word [ %02X %02X ]", resp.getSW1(), resp.getSW2());
			}
		}
		else
		{
			_ERR("select apdu is failed, rv [%d], length [%d]", ret, response.size());
		}

		return ret;
	}

	int FileObject::readBinary(unsigned int sfi, unsigned int length, ByteArray &result)
	{
		int ret;
		size_t offset = 0;
		ByteArray temp;

		do {
			ret = readBinary(sfi, offset, length - offset, temp);
			if (ret >= SCARD_ERROR_OK) {
				result += temp;
				offset += temp.size();
			}
		} while (ret >= SCARD_ERROR_OK && offset < length);

		return ret;
	}

	int FileObject::writeBinary(unsigned int sfi, const ByteArray &data, unsigned int offset, unsigned int length)
	{
		ByteArray command, response;
		APDUCommand apdu;
		int ret;

		apdu.setCommand(0, APDUCommand::INS_WRITE_BINARY, offset, 0, data, 0);
		apdu.getBuffer(command);

		ret = channel->transmitSync(command, response);
		if (ret == 0 && response.size() >= 2)
		{
			ResponseHelper resp(response);

			if (resp.getStatus() >= 0)
			{
				_DBG("response [%d] : %s", response.size(), response.toString().c_str());

				ret = SUCCESS;
			}
			else
			{
				_ERR("status word [ %02X %02X ]", resp.getSW1(), resp.getSW2());
			}
		}
		else
		{
			_ERR("select apdu is failed, rv [%d], length [%d]", ret, response.size());
		}

		return ret;
	}

	int FileObject::writeBinary(unsigned int sfi, const ByteArray &data)
	{
		int result;
		size_t offset = 0, len;
		ByteArray temp;

		do {
			len = MIN(data.size() - offset, MAX_SINGLE_LEN);
			temp.setBuffer(data.getBuffer(offset), len);
			result = writeBinary(sfi, temp, offset, len);
			if (result >= SCARD_ERROR_OK) {
				offset += len;
			}
		} while (result >= SCARD_ERROR_OK && offset < data.size());

		return result;
	}

	int FileObject::readBinaryAll(unsigned int sfi, ByteArray &result)
	{
		int ret;

		ret = readBinary(sfi, getFCP()->getFileSize(), result);

		return ret;
	}

} /* namespace smartcard_service_api */
