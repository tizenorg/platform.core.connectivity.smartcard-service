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


#ifndef SMARTCARD_TYPES_H_
#define SMARTCARD_TYPES_H_

#include <stdint.h>
#include <stdbool.h>

typedef void *se_service_h;
typedef void *reader_h;
typedef void *session_h;
typedef void *channel_h;

typedef void (*se_service_connected_cb)(se_service_h handle, void *context);
typedef void (*se_service_event_cb)(se_service_h handle, char *se_name, int event, void *context);
typedef void (*se_sesrvice_error_cb)(se_service_h handle, int error, void *context);

typedef void (*reader_open_session_cb)(session_h session, int error, void *user_data);

typedef void (*session_open_channel_cb)(channel_h channel, int error, void *user_data);
typedef void (*session_get_atr_cb)(unsigned char *atr, unsigned int length, int error, void *user_data);
typedef void (*session_close_session_cb)(int error, void *user_data);
typedef void (*session_get_channel_count_cb)(unsigned count, int error, void *user_data);

typedef void (*channel_transmit_cb)(unsigned char *buffer, unsigned int length, int error, void *user_data);
typedef void (*channel_close_cb)(int error, void *user_data);

#endif /* SMARTCARD_TYPES_H_ */
