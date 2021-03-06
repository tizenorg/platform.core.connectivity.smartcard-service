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

/* SLP library header */

/* local header */
#include "Debug.h"
#include "APDUHelper.h"

namespace smartcard_service_api
{
	/* ResponseHelper class */
	ResponseHelper::ResponseHelper()
	{
	}

	ResponseHelper::ResponseHelper(const ByteArray &response)
	{
		setResponse(response);
	}

	ResponseHelper::~ResponseHelper()
	{
	}

	bool ResponseHelper::setResponse(const ByteArray &response)
	{
		bool result = false;
		status = 0;
		dataField.releaseBuffer();

		this->response = response;

		if (response.getLength() >= 2)
		{
			sw[0] = response.getReverseAt(1);
			sw[1] = response.getReverseAt(0);

			status = parseStatusWord(sw);

			if (response.getLength() > 2)
			{
				dataField.setBuffer(response.getBuffer(), response.getLength() - 2);
			}

			result = true;
		}

		return result;
	}

	int ResponseHelper::parseStatusWord(unsigned char *sw)
	{
		int result = 0;

		switch (sw[0])
		{
		/* Normal processing */
		case (unsigned char)0x90 : /* SW2:00, No further qulification */
			break;

		case (unsigned char)0x91 : /* extra information */
			break;

		case (unsigned char)0x61 : /* SW2 encodes the number of data bytes still available */
			break;

		/* Warning processing */
		case (unsigned char)0x62 : /* State of non-volatile memory is unchanged (further qualification in SW2) */
			break;

		case (unsigned char)0x63 : /* State of non-volatile memory has changed (further qualification in SW2) */
			break;

		/* Execution error */
		case (unsigned char)0x64 : /* State of non-volatile memory is unchanged (further qualification in SW2) */
			result = -1;
			break;

		case (unsigned char)0x65 : /* State of non-volatile memory has changed (further qualification in SW2) */
			result = -1;
			break;

		case (unsigned char)0x66 : /* Security-related issues */
			result = -1;
			break;

		/* Checking error */
		case (unsigned char)0x67 : /* SW2:00, Wrong length; no further indication */
			result = -1;
			break;

		case (unsigned char)0x68 : /* Functions in CLA not supported (further qualification in SW2) */
			result = -1;
			break;

		case (unsigned char)0x69 : /* Command not allowed (further qualification in SW2) */
			result = -1;
			break;

		case (unsigned char)0x6A : /* Wrong parameters P1-P2 (further qualification in SW2) */
			result = -1;
			break;

		case (unsigned char)0x6B : /* SW2:00, Wrong parameters P1-P2 */
			result = -1;
			break;

		case (unsigned char)0x6C : /* Wrong Le field; SW2 encodes the exact number of available data bytes */
			result = -1;
			break;

		case (unsigned char)0x6D : /* SW2:00, Instruction code not supported or invalid */
			result = -1;
			break;

		case (unsigned char)0x6E : /* SW2:00, Class not supported */
			result = -1;
			break;

		case (unsigned char)0x6F : /* SW2:00, No precise diagnosis */
			result = -1;
			break;

		default :
			result = -1;
			break;
		}

		return result;
	}

	int ResponseHelper::getStatus()
	{
		return status;
	}

	int ResponseHelper::getStatus(const ByteArray &response)
	{
		int status = 0;

		if (response.getLength() >= 2)
		{
			status = ResponseHelper::parseStatusWord(response.getBuffer((response.getLength() - 2)));
		}

		return status;
	}

	ByteArray ResponseHelper::getDataField()
	{
		return dataField;
	}

	ByteArray ResponseHelper::getDataField(const ByteArray &response)
	{
		ByteArray result;

		if (response.getLength() > 2)
		{
			result.setBuffer(response.getBuffer(), response.getLength() - 2);
		}

		return result;
	}

	/* APDUCommand class */
	APDUCommand::APDUCommand()
	{
		maxResponseSize = 0;
		isExtendedLength = false;
		memset(&header, 0, sizeof(header));
	}

	APDUCommand::~APDUCommand()
	{
	}

	bool APDUCommand::setCommand(unsigned char cla, unsigned char ins, unsigned char p1, unsigned char p2, ByteArray commandData, unsigned int maxResponseSize)
	{
		setCLA(cla);
		setINS(ins);
		setP1(p1);
		setP2(p2);
		setCommandData(commandData);
		setMaxResponseSize(maxResponseSize);

		return true;
	}

	bool APDUCommand::setCommand(const ByteArray &command)
	{
		bool result = false;
		uint32_t offset = 0;
		uint32_t lengthSize = 1;

		if (command.getLength() < sizeof(header))
		{
			return false;
		}

		memcpy(&header, command.getBuffer(offset), sizeof(header));
		offset += sizeof(header);

		if (isExtendedLength)
		{
			lengthSize = 2;
		}

		if (command.getLength() - offset > lengthSize)
		{
			unsigned int length = 0;

			/* data exist */
			if (isExtendedLength)
			{
				/* TODO */
				offset += 2;
			}
			else
			{
				length = command.getAt(offset);
				offset += 1;
			}

			setCommandData(ByteArray(command.getBuffer(offset), length));
			offset += length;
		}

		if (command.getLength() - offset == lengthSize)
		{
			if (isExtendedLength)
			{
				/* TODO */
				offset += 2;
			}
			else
			{
				setMaxResponseSize(command.getAt(offset));
				offset += 1;
			}
		}

		if (command.getLength() == offset)
		{
			result = true;
		}
		else
		{
			SCARD_DEBUG_ERR("command stream is not correct, command.getLength() [%d], offset [%d]", command.getLength(), offset);
		}

		return result;
	}

	bool APDUCommand::setChannel(int type, int channelNum)
	{
		bool result = false;

		if (channelNum != 0)
		{
			switch (type)
			{
			case 0 :
				if (channelNum > 0 && channelNum < 4)
				{
					unsigned char temp;

					temp = getCLA();
					temp &= ~0x03;
					temp |= (channelNum & 0x03);
					setCLA(temp);

					result = true;
				}
				break;

			default :
				break;
			}
		}

		return result;
	}

	void APDUCommand::setCLA(unsigned char cla)
	{
		/* check criteria */
		if (cla == 0xFF)
			return;

		header.cla = cla;
	}

	unsigned char APDUCommand::getCLA()
	{
		return header.cla;
	}

	void APDUCommand::setINS(unsigned char ins)
	{
		/* check criteria */
		if ((ins & 0xF0) == 0x60 || (ins & 0xF0) == 0x90)
			return;

		header.ins = ins;
	}

	unsigned char APDUCommand::getINS()
	{
		return header.ins;
	}

	void APDUCommand::setP1(unsigned char p1)
	{
		/* check criteria */
		header.param[0] = p1;
	}

	unsigned char APDUCommand::getP1()
	{
		return header.param[0];
	}

	void APDUCommand::setP2(unsigned char p2)
	{
		/* check criteria */
		header.param[1] = p2;
	}

	unsigned char APDUCommand::getP2()
	{
		return header.param[1];
	}

	void APDUCommand::setCommandData(const ByteArray &data)
	{
		commandData = data;
	}

	ByteArray APDUCommand::getCommandData()
	{
		return commandData;
	}

	void APDUCommand::setMaxResponseSize(unsigned int maxResponseSize)
	{
		this->maxResponseSize = maxResponseSize;
	}

	unsigned int APDUCommand::setMaxResponseSize()
	{
		return maxResponseSize;
	}

	bool APDUCommand::getBuffer(ByteArray &array)
	{
		unsigned char *temp_buffer = NULL;
		unsigned int temp_len = 0;
		unsigned char lc[3] = { 0, };
		unsigned int lc_len = 0;
		unsigned char le[3] = { 0, };
		unsigned int le_len = 0;
		unsigned int offset = 0;

		/* */
		temp_len += sizeof(header);

		/* calculate lc length */
		if (commandData.getLength() > 0)
		{
			if (isExtendedLength/*commandData.getLength() > 255*/)
			{
				lc[1] = (commandData.getLength() >> 8) & 0x000000FF;
				lc[2] = commandData.getLength() & 0x000000FF;

				lc_len = 3;
			}
			else
			{
				lc[0] = commandData.getLength() & 0x000000FF;

				lc_len = 1;
			}
		}

		temp_len += lc_len;

		/* add command data length */
		temp_len += commandData.getLength();

		/* calculate le length */
		if (maxResponseSize > 0)
		{
			if (isExtendedLength/*commandData.getLength() > 255*/)
			{
				if (maxResponseSize < 65536)
				{
					le[1] = (maxResponseSize >> 8) & 0x000000FF;
					le[2] = maxResponseSize & 0x000000FF;

					le_len = 3;
				}
				else if (maxResponseSize == 65536)
				{
					le_len = 2;
				}
			}
			else
			{
				if (maxResponseSize != 256)
					le[0] = maxResponseSize & 0x000000FF;

				le_len = 1;
			}
		}

		temp_len += le_len;

		temp_buffer = new unsigned char[temp_len];
		if (temp_buffer == NULL)
			return false;

		/* fill data */
		offset = 0;

		memcpy(temp_buffer + offset, &header, sizeof(header));
		offset += sizeof(header);

		if (commandData.getLength() > 0)
		{
			memcpy(temp_buffer + offset, &lc, lc_len);
			offset += lc_len;

			memcpy(temp_buffer + offset, commandData.getBuffer(), commandData.getLength());
			offset += commandData.getLength();
		}

		if (maxResponseSize > 0)
		{
			memcpy(temp_buffer + offset, &le, le_len);
			offset += le_len;
		}

		array.setBuffer(temp_buffer, temp_len);
		delete []temp_buffer;

		return true;
	}

	/* APDUHelper class */
	ByteArray APDUHelper::generateAPDU(int command, int channel, ByteArray data)
	{
		ByteArray result;
		APDUCommand apdu;

		switch (command)
		{
		case COMMAND_OPEN_LOGICAL_CHANNEL :
			apdu.setCommand(0, APDUCommand::INS_MANAGE_CHANNEL, 0, 0, ByteArray::EMPTY, 1);
			apdu.getBuffer(result);
			break;

		case COMMAND_CLOSE_LOGICAL_CHANNEL :
			apdu.setCommand(0, APDUCommand::INS_MANAGE_CHANNEL, 0x80, channel, ByteArray::EMPTY, 0);
			apdu.getBuffer(result);
			break;

		case COMMAND_SELECT_BY_ID :
			apdu.setCommand(0, APDUCommand::INS_SELECT_FILE, APDUCommand::P1_SELECT_BY_ID, APDUCommand::P2_SELECT_GET_FCP, data, 0);
			apdu.getBuffer(result);
			break;

		case COMMAND_SELECT_PARENT_DF :
			apdu.setCommand(0, APDUCommand::INS_SELECT_FILE, APDUCommand::P1_SELECT_PARENT_DF, APDUCommand::P2_SELECT_GET_FCP, data, 0);
			apdu.getBuffer(result);
			break;

		case COMMAND_SELECT_BY_DF_NAME :
			apdu.setCommand(0, APDUCommand::INS_SELECT_FILE, APDUCommand::P1_SELECT_BY_DF_NAME, APDUCommand::P2_SELECT_GET_FCP, data, 0);
			apdu.getBuffer(result);
			break;

		case COMMAND_SELECT_BY_PATH :
			apdu.setCommand(0, APDUCommand::INS_SELECT_FILE, APDUCommand::P1_SELECT_BY_PATH, APDUCommand::P2_SELECT_GET_FCP, data, 0);
			apdu.getBuffer(result);
			break;

		case COMMAND_SELECT_BY_PATH_FROM_CURRENT_DF :
			apdu.setCommand(0, APDUCommand::INS_SELECT_FILE, APDUCommand::P1_SELECT_BY_PATH_FROM_CURRENT_DF, APDUCommand::P2_SELECT_GET_FCP, data, 0);
			apdu.getBuffer(result);
			break;

		default :
			break;
		}

		return result;
	}

} /* namespace smartcard_service_api */
