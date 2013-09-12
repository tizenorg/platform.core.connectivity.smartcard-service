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

#ifndef RECORD_H_
#define RECORD_H_

#include "ByteArray.h"

namespace smartcard_service_api
{
	class Record
	{
	private:
		unsigned int id;
		ByteArray data;

	public:
		Record() : id(0) {}
		Record(unsigned int id, const ByteArray &buffer) : id(id),
			data(buffer) {};
		~Record() {}

		inline unsigned int getID() const { return id; }
		inline const ByteArray getData() const { return data; }
	};

} /* namespace smartcard_service_api */
#endif /* RECORD_H_ */
