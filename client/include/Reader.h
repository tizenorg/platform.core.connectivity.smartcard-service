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

#ifndef READER_H_
#define READER_H_

/* standard library header */
#include <glib.h>
#include <gio/gio.h>

/* SLP library header */

/* local header */
#include "smartcard-types.h"
#ifdef __cplusplus
#include "ReaderHelper.h"
#include "Session.h"
#endif /* __cplusplus */

#ifdef __cplusplus
namespace smartcard_service_api
{
	class Reader: public ReaderHelper
	{
	private:
		void *context;
		void *handle;
		void *proxy;

		Reader(void *context, const char *name, void *handle);
		~Reader();

		inline void unavailable() { present = false; }
		static void reader_open_session_cb(GObject *source_object,
			GAsyncResult *res, gpointer user_data);

	public:
		void closeSessions()
			throw(ErrorIO &, ErrorIllegalState &);

		int openSession(openSessionCallback callback, void *userData);
		SessionHelper *openSessionSync()
			throw(ExceptionBase &, ErrorIO &, ErrorIllegalState &,
				ErrorIllegalParameter &, ErrorSecurity &);

		friend class SEService;
	};
} /* namespace smartcard_service_api */
#endif /* __cplusplus */

/* export C API */
#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

const char *reader_get_name(reader_h handle);
se_service_h reader_get_se_service(reader_h handle);
bool reader_is_secure_element_present(reader_h handle);
int reader_open_session(reader_h handle, reader_open_session_cb callback,
	void *userData);
session_h reader_open_session_sync(reader_h handle);
void reader_close_sessions(reader_h handle);
__attribute__((deprecated)) void reader_destroy_instance(reader_h handle);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* READER_H_ */
