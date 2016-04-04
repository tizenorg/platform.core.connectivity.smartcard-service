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

/* standard library header */
#include <glib.h>

/* SLP library header */

/* local header */
#include "smartcard-types.h"
#include "Debug.h"
#include "ByteArray.h"
#include "ClientGDBus.h"

using namespace std;

/* below functions will be called when dlopen or dlclose is called */
void __attribute__ ((constructor)) lib_init()
{
	/* remove for deprecated-declarations build warning: glib ver > 2.36 */
	/* g_type_init(); */
}

void __attribute__ ((destructor)) lib_fini()
{
}

namespace smartcard_service_api
{
} /* namespace smartcard_service_api */
