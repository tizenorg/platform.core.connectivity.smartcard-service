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


#ifndef CLIENTCHANNEL_H_
#define CLIENTCHANNEL_H_

/* standard library header */

/* SLP library header */

/* local header */
#include "smartcard-types.h"
#ifdef __cplusplus
#include "Channel.h"
#include "Session.h"
#endif /* __cplusplus */

#ifdef __cplusplus
namespace smartcard_service_api
{
	class ClientChannel: public Channel
	{
	private:
		void *context;
		void *handle;
		/* temporary data for sync function */
		int error;
		ByteArray response;

		ClientChannel(void *context, Session *session, int channelNum, ByteArray selectResponse, void *handle);

		static bool dispatcherCallback(void *message);

		void closeSync();
		int transmitSync(ByteArray command, ByteArray &result);

	public:
		~ClientChannel();

		int close(closeCallback callback, void *userParam);
		int transmit(ByteArray command, transmitCallback callback, void *userParam);

		friend class ClientDispatcher;
		friend class Session;
	};

} /* namespace smartcard_service_api */
#endif /* __cplusplus */

/* export C API */
#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

int channel_close(channel_h handle, channel_close_cb callback, void *userParam);
int channel_transmit(channel_h handle, unsigned char *command, unsigned int length, channel_transmit_cb callback, void *userParam);
bool channel_is_basic_channel(channel_h handle);
bool channel_is_closed(channel_h handle);

unsigned int channel_get_select_response_length(channel_h handle);
bool channel_get_select_response(channel_h handle, unsigned char *buffer, unsigned int length);
session_h channel_get_session(channel_h handle);
void channel_destroy_instance(channel_h handle);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CLIENTCHANNEL_H_ */
