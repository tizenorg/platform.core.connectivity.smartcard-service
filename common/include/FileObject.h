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


#ifndef FILEOBJECT_H_
#define FILEOBJECT_H_

/* standard library header */
#include <vector>

/* SLP library header */

/* local header */
#include "ProviderHelper.h"
#include "ByteArray.h"
#include "FCI.h"
#include "Record.h"

using namespace std;

namespace smartcard_service_api
{
	class FileObject : public ProviderHelper
	{
	private:
		FCI fci;
		FCP fcp;
		bool opened;

		int _select(ByteArray command);

	protected:
		ByteArray selectResponse;
		bool setSelectResponse(ByteArray &response);

	public:
		static const int SUCCESS = 0;
		static const int ERROR_ILLEGAL_STATE = -1;
		static const int ERROR_ILLEGAL_REFERENCE = -2;
		static const int ERROR_ILLEGAL_PARAMETER = -3;
		static const int ERROR_SECURITY = -4;
		static const int ERROR_OPERATION_NOT_SUPPORT = -5;
		static const int ERROR_IO = -6;
		static const int ERROR_UNKNOWN = -99;

		FileObject(Channel *channel);
		FileObject(Channel *channel, ByteArray selectResponse);
		~FileObject();

		inline bool isClosed() { return (opened == false); }
		int select(ByteArray aid);
		int select(ByteArray path, bool fromCurrentDF);
		int select(unsigned int fid);
		int selectParent();

		FCI *getFCI();
		FCP *getFCP();

		int readRecord(unsigned int sfi, unsigned int recordId, Record &result);
		int writeRecord(unsigned int sfi, Record record);

		int searchRecord(unsigned int sfi, ByteArray searchParam, vector<int> &result);

		int readBinary(unsigned int sfi, unsigned int offset, unsigned int length, ByteArray &result);
		int writeBinary(unsigned int sfi, ByteArray data, unsigned int offset, unsigned int length);
	};

} /* namespace smartcard_service_api */
#endif /* FILEOBJECT_H_ */
