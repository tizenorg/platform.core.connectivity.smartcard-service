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

#ifndef TERMINALINTERFACE_H_
#define TERMINALINTERFACE_H_

/* standard library header */

/* SLP library header */

/* local header */

//typedef unsigned int terminal_handle_h;
//
//typedef enum
//{
//	TERMINAL_SUCCESS = 0,
//
//	TERMINAL_ERROR_UNKNOWN = -1,
//	/* TERMINAL_ERROR_ = -, */
//} terminal_result_e;
//
//typedef char *(*terminal_get_name_fn)();
//typedef terminal_result_e (*terminal_initialize_fn)(int param1, char *param2);
//typedef terminal_result_e (*terminal_finalize_fn)(int param1, char *param2);
//typedef terminal_result_e (*terminal_open_fn)(int param1, char *param2, terminal_handle_h *handle);
//typedef terminal_result_e (*terminal_close_fn)(terminal_handle_h handle);
//typedef terminal_result_e (*terminal_transmit_fn)(terminal_handle_h handle, unsigned char *command, unsigned int cmd_len, unsigned char *response, unsigned int *resp_len);
//typedef terminal_result_e (*terminal_get_atr_fn)(terminal_handle_h handle, unsigned char *atr, unsigned int *atr_len);
//
//typedef struct _terminal_interfaces_t
//{
//	terminal_get_name_fn api_get_name;
//	terminal_initialize_fn api_initialize;
//	terminal_finalize_fn api_finalize;
//	terminal_open_fn api_open;
//	terminal_close_fn api_close;
//	terminal_transmit_fn api_transmit;
//	terminal_get_atr_fn api_get_atr;
//} terminal_interfaces_t;
//
//typedef int (*terminal_get_interfaces_fn)(terminal_interfaces_t *apis);

typedef const char *(*terminal_get_name_fn)();
typedef void *(*terminal_create_instance_fn)();
typedef void (*terminal_destroy_instance_fn)(void *);

#endif /* TERMINALINTERFACE_H_ */
