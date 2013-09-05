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

#ifndef GDBUSHELPER_H_
#define GDBUSHELPER_H_

#include <glib.h>

#include "ByteArray.h"

namespace smartcard_service_api
{
	class CallbackParam
	{
	public :
		void *instance;
		void *callback;
		void *user_param;
	};

	class GDBusHelper
	{
	public :
		static void convertVariantToByteArray(GVariant *var,
			ByteArray &array);

		static GVariant *convertByteArrayToVariant(
			const ByteArray &array);
	};
} /* namespace smartcard_service_api */

#endif /* GDBUSHELPER_H_ */
