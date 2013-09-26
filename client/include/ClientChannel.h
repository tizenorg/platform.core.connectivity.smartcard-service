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

#ifndef CLIENTCHANNEL_H_
#define CLIENTCHANNEL_H_

#ifdef USE_AUTOSTART
#include <gio/gio.h>
#endif

#include "smartcard-types.h"
#ifdef __cplusplus
#include "Channel.h"
#include "Session.h"
#endif /* __cplusplus */

#ifdef __cplusplus
namespace smartcard_service_api
{
	class EXPORT ClientChannel: public Channel
	{
	private:
		void *context;
		void *handle;
		void *proxy;

		ClientChannel(void *context, Session *session, int channelNum,
			const ByteArray &selectResponse, void *handle);
		~ClientChannel();
#ifdef USE_AUTOSTART
		static void channel_transmit_cb(GObject *source_object,
			GAsyncResult *res, gpointer user_data);
		static void channel_close_cb(GObject *source_object,
			GAsyncResult *res, gpointer user_data);
#endif
	public:
		int close(closeChannelCallback callback, void *userParam);
		int transmit(const ByteArray &command,
			transmitCallback callback, void *userParam);

		void closeSync()
			throw(ExceptionBase &, ErrorIO &, ErrorIllegalState &,
				ErrorSecurity &, ErrorIllegalParameter &);
		int transmitSync(const ByteArray &command, ByteArray &result)
			throw(ExceptionBase &, ErrorIO &, ErrorIllegalState &,
				ErrorIllegalParameter &, ErrorSecurity &);

		friend class Session;
	};
} /* namespace smartcard_service_api */
#endif /* __cplusplus */

/* export C API */
#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

bool channel_is_basic_channel(channel_h handle);
bool channel_is_closed(channel_h handle);

unsigned int channel_get_select_response_length(channel_h handle);
bool channel_get_select_response(channel_h handle, unsigned char *buffer,
	unsigned int length);
session_h channel_get_session(channel_h handle);
void channel_destroy_instance(channel_h handle) __attribute__((deprecated));

int channel_close(channel_h handle, channel_close_cb callback, void *userParam);
int channel_transmit(channel_h handle, unsigned char *command,
	unsigned int length, channel_transmit_cb callback, void *userParam);
void channel_close_sync(channel_h handle);
int channel_transmit_sync(channel_h handle, unsigned char *command,
	unsigned int cmd_len, unsigned char **response, unsigned int *resp_len);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CLIENTCHANNEL_H_ */
