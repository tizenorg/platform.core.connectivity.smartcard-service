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

#ifndef SESSION_H_
#define SESSION_H_

/* standard library header */
#ifdef USE_AUTOSTART
#include <gio/gio.h>
#endif

/* SLP library header */

/* local header */
#include "smartcard-types.h"
#ifdef __cplusplus
#include "SessionHelper.h"
#endif /* __cplusplus */

#ifdef __cplusplus
namespace smartcard_service_api
{
	class Reader;

	class Session : public SessionHelper
	{
	private:
		void *context;
		void *handle;
		void *proxy;

		Session(void *context, Reader *reader, void *handle);
		~Session();

		int openChannel(int id, const ByteArray &aid, openChannelCallback callback, void *userData);
		Channel *openChannelSync(int id, const ByteArray &aid)
			throw(ExceptionBase &, ErrorIO &, ErrorIllegalState &,
				ErrorIllegalParameter &, ErrorSecurity &);

#ifdef USE_AUTOSTART
		static void session_get_atr_cb(GObject *source_object,
			GAsyncResult *res, gpointer user_data);
		static void session_open_channel_cb(GObject *source_object,
			GAsyncResult *res, gpointer user_data);
		static void session_close_cb(GObject *source_object,
			GAsyncResult *res, gpointer user_data);
#endif

	public:
		void closeChannels()
			throw(ErrorIO &, ErrorIllegalState &);

		int getATR(getATRCallback callback, void *userData);
		int close(closeSessionCallback callback, void *userData);

		int openBasicChannel(const ByteArray &aid, openChannelCallback callback, void *userData);
		int openBasicChannel(const unsigned char *aid, unsigned int length, openChannelCallback callback, void *userData);
		int openLogicalChannel(const ByteArray &aid, openChannelCallback callback, void *userData);
		int openLogicalChannel(const unsigned char *aid, unsigned int length, openChannelCallback callback, void *userData);

		const ByteArray getATRSync()
			throw(ExceptionBase &, ErrorIO &, ErrorSecurity &,
				ErrorIllegalState &, ErrorIllegalParameter &);

		void closeSync()
			throw(ExceptionBase &, ErrorIO &, ErrorSecurity &,
				ErrorIllegalState &, ErrorIllegalParameter &);

		Channel *openBasicChannelSync(const ByteArray &aid)
			throw(ExceptionBase &, ErrorIO &, ErrorIllegalState &,
				ErrorIllegalParameter &, ErrorSecurity &);

		Channel *openBasicChannelSync(const unsigned char *aid, unsigned int length)
			throw(ExceptionBase &, ErrorIO &, ErrorIllegalState &,
				ErrorIllegalParameter &, ErrorSecurity &);

		Channel *openLogicalChannelSync(const ByteArray &aid)
			throw(ExceptionBase &, ErrorIO &, ErrorIllegalState &,
				ErrorIllegalParameter &, ErrorSecurity &);

		Channel *openLogicalChannelSync(const unsigned char *aid, unsigned int length)
			throw(ExceptionBase &, ErrorIO &, ErrorIllegalState &,
				ErrorIllegalParameter &, ErrorSecurity &);

		size_t getChannelCount() const;

		friend class Reader;
	};
} /* namespace smartcard_service_api */
#endif /* __cplusplus */

/* export C API */
#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

reader_h session_get_reader(session_h handle);
bool session_is_closed(session_h handle);
__attribute__((deprecated)) void session_destroy_instance(session_h handle);
void session_close_channels(session_h handle);

int session_get_atr(session_h handle, session_get_atr_cb callback,
	void *userData);
int session_close(session_h handle, session_close_session_cb callback,
	void *userData);

int session_open_basic_channel(session_h handle, unsigned char *aid,
	unsigned int length, session_open_channel_cb callback, void *userData);
int session_open_logical_channel(session_h handle, unsigned char *aid,
	unsigned int length, session_open_channel_cb callback, void *userData);

int session_get_atr_sync(session_h handle, unsigned char **buffer,
	unsigned int *length);
void session_close_sync(session_h handle);

channel_h session_open_basic_channel_sync(session_h handle,
	unsigned char *aid, unsigned int length);
channel_h session_open_logical_channel_sync(session_h handle,
	unsigned char *aid, unsigned int length);

size_t session_get_channel_count(session_h handle);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* SESSION_H_ */
