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

#ifndef APDUHELPER_H_
#define APDUHELPER_H_

#include "Debug.h"
#include "ByteArray.h"

namespace smartcard_service_api
{
	class EXPORT ResponseHelper
	{
	private:
		ByteArray response;
		unsigned char sw[2];
		int status;
		ByteArray dataField;

		static int parseStatusWord(const unsigned char *sw);

	public:
		static const int SUCCESS = 0;

		static const int ERROR_UNKNOWN = -1;

		static const int ERROR_NO_INFORMATION = -(0x6900);
		static const int ERROR_COMMAND_INCOMPATIBLE = -(0x6981);
		static const int ERROR_SECURITY_NOT_SATISFIED = -(0x6982);
		static const int ERROR_AUTH_PIN_BLOCKED = -(0x6983);
		static const int ERROR_REF_DATA_INVALID = -(0x6984);
		static const int ERROR_CONDITION_NOT_SATIFIED = -(0x6985);
		static const int ERROR_COMMAND_NOT_ALLOW = -(0x6986);

		static const int ERROR_INCORRECT_PARAMETER = -(0x6a80);
		static const int ERROR_FUNCTION_NOT_SUPPORT = -(0x6a81);
		static const int ERROR_FILE_NOT_FOUND = -(0x6a82);
		static const int ERROR_RECORD_NOT_FOUND = -(0x6a83);
		static const int ERROR_NOT_ENOUGH_MEMORY = -(0x6a84);
		static const int ERROR_INCORRECT_P1_P2 = -(0x6a86);
		static const int ERROR_LC_INCONSISTENT = -(0x6a87);
		static const int ERROR_REF_DATA_NOT_FOUND = -(0x6a88);

		ResponseHelper();
		ResponseHelper(const ByteArray &response);
		~ResponseHelper();

		bool setResponse(const ByteArray &response);
		inline int getStatus() const { return status; }
		inline unsigned char getSW1() const { return sw[0]; }
		inline unsigned char getSW2() const { return sw[1]; }

//		char *getErrorString();
		inline const ByteArray getDataField() const { return dataField; }

		static int getStatus(const ByteArray &response);
		static const ByteArray getDataField(const ByteArray &response);
//		static char *getErrorString();
	};

	class EXPORT APDUCommand
	{
	private:
		typedef struct _command_header_t
		{
			unsigned char cla;
			unsigned char ins;
			unsigned char param[2];
		} command_header_t;

		command_header_t header;
		ByteArray commandData;
		unsigned int maxResponseSize;
		bool isExtendedLength;

	public:
		static const unsigned char INS_DEACTIVATE_FILE = (unsigned char)0x04;
		static const unsigned char INS_TERMINAL_PROFILE = (unsigned char)0x10;
		static const unsigned char INS_FETCH = (unsigned char)0x12;
		static const unsigned char INS_TERMINAL_RESPONSE = (unsigned char)0x14;
		static const unsigned char INS_VERIFY = (unsigned char)0x20;
		static const unsigned char INS_CHANGE_PIN = (unsigned char)0x24;
		static const unsigned char INS_DISABLE_PIN = (unsigned char)0x26;
		static const unsigned char INS_ENABLE_PIN = (unsigned char)0x28;
		static const unsigned char INS_UNBLOCK_PIN = (unsigned char)0x2C;
		static const unsigned char INS_INCREASE = (unsigned char)0x32;
		static const unsigned char INS_ACTIVATE_FILE = (unsigned char)0x44;
		static const unsigned char INS_GET_CHALLENGE = (unsigned char)0x84;
		static const unsigned char INS_AUTHENTICATE = (unsigned char)0x88;
		static const unsigned char INS_AUTHENTICATE2 = (unsigned char)0x89;
		static const unsigned char INS_MANAGE_CHANNEL = (unsigned char)0x70;
		static const unsigned char INS_MANAGE_SECURE_CHANNEL = (unsigned char)0x73;
		static const unsigned char INS_TRANSACT_DATA = (unsigned char)0x75;
		static const unsigned char INS_SEARCH_RECORD = (unsigned char)0xA2;
		static const unsigned char INS_SELECT_FILE = (unsigned char)0xA4;
		static const unsigned char INS_TERMINAL_CAPABILITY = (unsigned char)0xAA;
		static const unsigned char INS_READ_BINARY = (unsigned char)0xB0;
		static const unsigned char INS_READ_RECORD = (unsigned char)0xB2;
		static const unsigned char INS_GET_RESPONSE = (unsigned char)0xC0;
		static const unsigned char INS_ENVELOPE = (unsigned char)0xC2;
		static const unsigned char INS_RETRIEVE_DATA = (unsigned char)0xCB;
		static const unsigned char INS_WRITE_BINARY = (unsigned char)0xD0;
		static const unsigned char INS_UPDATE_BINARY = (unsigned char)0xD6;
		static const unsigned char INS_UPDATE_RECORD = (unsigned char)0xDC;
		static const unsigned char INS_SET_DATA = (unsigned char)0xDB;
		static const unsigned char INS_CREATE_FILE = (unsigned char)0xE0;
		static const unsigned char INS_APPEND_RECORD = (unsigned char)0xE2;
		static const unsigned char INS_DELETE_FILE = (unsigned char)0xE4;
		static const unsigned char INS_STATUS = (unsigned char)0xF2;
//		static const unsigned char INS_ = (unsigned char)0x;

		static const unsigned char P1_SELECT_BY_ID = (unsigned char)0x00;
		static const unsigned char P1_SELECT_PARENT_DF = (unsigned char)0x03;
		static const unsigned char P1_SELECT_BY_DF_NAME = (unsigned char)0x04;
		static const unsigned char P1_SELECT_BY_PATH = (unsigned char)0x08;
		static const unsigned char P1_SELECT_BY_PATH_FROM_CURRENT_DF = (unsigned char)0x09;
//		static const unsigned char P1_ = (unsigned char)0x;

		static const unsigned char P2_SELECT_GET_FCP = (unsigned char)0x04;
//		static const unsigned char P2_ = (unsigned char)0x;

		static const unsigned char CLA_CHANNEL_STANDARD = (unsigned char)0x00;
		static const unsigned char CLA_CHANNEL_EXTENDED = (unsigned char)0x01;

		static const unsigned int LE_MAX = -1;

		APDUCommand();
		~APDUCommand();

		bool setCommand(unsigned char cla, unsigned char ins, unsigned char p1,
			unsigned char p2, const ByteArray &commandData, unsigned int maxResponseSize);
		bool setCommand(const ByteArray &command);

		bool setChannel(int type, int channelNum);

		void setCLA(unsigned char cla);
		unsigned char getCLA() const;

		void setINS(unsigned char ins);
		unsigned char getINS() const;

		void setP1(unsigned char p1);
		unsigned char getP1() const;

		void setP2(unsigned char p2);
		unsigned char getP2() const;

		void setCommandData(const ByteArray &data);
		const ByteArray getCommandData() const;

		void setMaxResponseSize(unsigned int maxResponseSize);
		unsigned int getMaxResponseSize() const;

		bool getBuffer(ByteArray &array) const;
	};

	class EXPORT APDUHelper
	{
	public:
		static const int COMMAND_OPEN_LOGICAL_CHANNEL = 1;
		static const int COMMAND_CLOSE_LOGICAL_CHANNEL = 2;
		static const int COMMAND_SELECT_BY_ID = 3;
		static const int COMMAND_SELECT_PARENT_DF = 4;
		static const int COMMAND_SELECT_BY_DF_NAME = 5;
		static const int COMMAND_SELECT_BY_PATH = 6;
		static const int COMMAND_SELECT_BY_PATH_FROM_CURRENT_DF = 7;
		static const int COMMAND_READ_BINARY = 8;
		static const int COMMAND_READ_RECORD = 9;
		static const int COMMAND_WRITE_BINARY = 10;
		static const int COMMAND_WRITE_RECORD = 11;

		static const ByteArray generateAPDU(int command,
			int channel, const ByteArray &data);
	};

} /* namespace smartcard_service_api */
#endif /* APDUHELPER_H_ */
