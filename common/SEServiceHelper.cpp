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

/* SLP library header */

/* local header */
#include "SEServiceHelper.h"

namespace smartcard_service_api
{
	SEServiceHelper::SEServiceHelper()
	{
		connected = false;
	}

	SEServiceHelper::~SEServiceHelper()
	{
		shutdown();
	}

	vector<ReaderHelper *> SEServiceHelper::getReaders()
	{
		return readers;
	}

	bool SEServiceHelper::isConnected()
	{
		return (readers.size() > 0);
	}

	void SEServiceHelper::shutdown()
	{
		uint32_t i;

		for (i = 0; i < readers.size(); i++)
		{
			readers[i]->closeSessions();
		}

		readers.clear();
	}

} /* namespace smartcard_service_api */
