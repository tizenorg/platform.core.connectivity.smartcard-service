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


#ifndef SERIALIZABLE_H_
#define SERIALIZABLE_H_

#include "ByteArray.h"

namespace smartcard_service_api
{
	class Serializable
	{
		virtual ByteArray serialize() = 0;
		virtual void deserialize(ByteArray buffer) = 0;
	};

} /* namespace smartcard_service_api */
#endif /* SERIALIZABLE_H_ */
