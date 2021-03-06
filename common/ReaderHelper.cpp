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
#include "ReaderHelper.h"

namespace smartcard_service_api
{
	ReaderHelper::ReaderHelper()
	{
		memset(name, 0, sizeof(name));
		seService = NULL;
	}

	ReaderHelper::~ReaderHelper()
	{
	}

	const char *ReaderHelper::getName()
	{
		return (const char *)name;
	}

	SEServiceHelper *ReaderHelper::getSEService()
	{
		return seService;
	}

	bool ReaderHelper::isSecureElementPresent()
	{
		/* get checkse() symbol of se library (dlsym) */
		/* invoke checkse() and return result */

		return true /* checkse() */;
	}

} /* namespace smartcard_service_api */
