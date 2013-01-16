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


#ifndef PROVIDERHELPER_H_
#define PROVIDERHELPER_H_

/* standard library header */

/* SLP library header */

/* local header */
#include "Channel.h"

namespace smartcard_service_api
{
	class ProviderHelper
	{
	protected:
		Channel *channel;

//		ProviderHelper();

	public:
		ProviderHelper(Channel *channel);
		~ProviderHelper();

		Channel *getChannel();
	};

} /* namespace smartcard_service_api */
#endif /* PROVIDERHELPER_H_ */
