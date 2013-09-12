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

#ifndef GPARAM_H_
#define GPARAM_H_

/* standard library header */
#include <vector>

/* SLP library header */

/* local header */
#include "FileObject.h"

using namespace std;

namespace smartcard_service_api
{
	class GPARAM : public FileObject
	{
	public:
		GPARAM(Channel *channel);
		GPARAM(Channel *channel, const ByteArray &selectResponse);
		~GPARAM() {}

		int select();

		int getDataAll(ByteArray &data);
		int getDataSpecific(const ByteArray &aid, const ByteArray &hash, ByteArray &data);
		int getDataRefreshTag(ByteArray &tag);

		/* override not supported functions */
		int select(const ByteArray &aid) { return SCARD_ERROR_NOT_SUPPORTED; }
		int select(const ByteArray &path, bool fromCurrentDF) { return SCARD_ERROR_NOT_SUPPORTED; }
		int select(unsigned int fid) { return SCARD_ERROR_NOT_SUPPORTED; }
		int selectParent() { return SCARD_ERROR_NOT_SUPPORTED; }
		int readRecord(unsigned int sfi, unsigned int recordId, Record &result) { return SCARD_ERROR_NOT_SUPPORTED; }
		int writeRecord(unsigned int sfi, const Record &record) { return SCARD_ERROR_NOT_SUPPORTED; }
		int searchRecord(unsigned int sfi, const ByteArray &searchParam, vector<int> &result) { return SCARD_ERROR_NOT_SUPPORTED; }
		int readBinary(unsigned int sfi, unsigned int offset, unsigned int length, ByteArray &result) { return SCARD_ERROR_NOT_SUPPORTED; }
		int writeBinary(unsigned int sfi, const ByteArray &data, unsigned int offset, unsigned int length) { return SCARD_ERROR_NOT_SUPPORTED; }
	};

} /* namespace smartcard_service_api */
#endif /* GPARAM_H_ */
