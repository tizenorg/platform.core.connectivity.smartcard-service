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

#ifndef SESERVICELISTENER_H_
#define SESERVICELISTENER_H_

#include "Debug.h"

namespace smartcard_service_api
{
	class SEServiceHelper;

	class EXPORT SEServiceListener
	{
	public:
		virtual void serviceConnected(SEServiceHelper *service,
			void *context) = 0;
		virtual void eventHandler(SEServiceHelper *service,
			char *seName, int event, void *context) = 0;
		virtual void errorHandler(SEServiceHelper *service, int error,
			void *context) = 0;
	};
} /* namespace open_mobile_api */

#endif /* SESERVICELISTENER_H_ */
