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

#ifndef EFDIR_H_
#define EFDIR_H_

#include <vector>

#include "FileObject.h"

using namespace std;

namespace smartcard_service_api
{
	class EFDIR : public FileObject
	{
	private:
		const ByteArray parseRecord(const Record &record, const ByteArray &aid);

	public:
		EFDIR(Channel *channel);
		EFDIR(Channel *channel, const ByteArray &selectResponse);
		~EFDIR();

		int select();

		const ByteArray getPathByAID(const ByteArray &aid);
	};
} /* namespace smartcard_service_api */
#endif /* EFDIR_H_ */
