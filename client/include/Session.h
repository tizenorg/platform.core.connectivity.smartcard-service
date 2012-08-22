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


#ifndef SESSION_H_
#define SESSION_H_

/* standard library header */

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

	class Session: public SessionHelper
	{
	private:
		void *context;
		void *handle;
		/* temporary data for sync function */
		int error;
		Channel *openedChannel;
		unsigned int channelCount;

		Session(void *context, Reader *reader, void *handle);

		int openChannel(int id, ByteArray aid, openChannelCallback callback, void *userData);
		static bool dispatcherCallback(void *message);

		Channel *openChannelSync(int id, ByteArray aid);

		ByteArray getATRSync();
		void closeSync();

		Channel *openBasicChannelSync(ByteArray aid);
		Channel *openBasicChannelSync(unsigned char *aid, unsigned int length);
		Channel *openLogicalChannelSync(ByteArray aid);
		Channel *openLogicalChannelSync(unsigned char *aid, unsigned int length);
		unsigned int getChannelCountSync();

	public:
		~Session();

		int getATR(getATRCallback callback, void *userData);
		int close(closeSessionCallback callback, void *userData);

		void closeChannels();

		int openBasicChannel(ByteArray aid, openChannelCallback callback, void *userData);
		int openBasicChannel(unsigned char *aid, unsigned int length, openChannelCallback callback, void *userData);
		int openLogicalChannel(ByteArray aid, openChannelCallback callback, void *userData);
		int openLogicalChannel(unsigned char *aid, unsigned int length, openChannelCallback callback, void *userData);
		unsigned int getChannelCount(getChannelCountCallback callback, void * userData);

		friend class ClientDispatcher;
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
int session_get_atr(session_h handle, session_get_atr_cb callback, void *userData);
int session_close(session_h handle, session_close_session_cb callback, void *userData);
bool session_is_closed(session_h handle);
void session_close_channels(session_h handle);

int session_open_basic_channel(session_h handle, unsigned char *aid, unsigned int length, session_open_channel_cb callback, void *userData);
int session_open_logical_channel(session_h handle, unsigned char *aid, unsigned int length, session_open_channel_cb callback, void *userData);
unsigned int session_get_channel_count(session_h handle, session_get_channel_count_cb callback, void * userData);
void session_destroy_instance(session_h handle);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* SESSION_H_ */
