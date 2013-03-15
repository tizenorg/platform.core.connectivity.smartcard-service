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

#ifndef DEBUG_H_
#define DEBUG_H_

/* standard library header */

/* SLP library header */
#include "dlog.h"

/* local header */

#define COLOR_RED 		"\033[0;31m"
#define COLOR_GREEN 	"\033[0;32m"
#define COLOR_BROWN 	"\033[0;33m"
#define COLOR_BLUE 		"\033[0;34m"
#define COLOR_PURPLE 	"\033[0;35m"
#define COLOR_CYAN 		"\033[0;36m"
#define COLOR_LIGHTBLUE "\033[0;37m"
#define COLOR_END		"\033[0;m"

#define SCARD_DEBUG(fmt, ...)\
	do\
	{\
		LOGD(fmt, ##__VA_ARGS__);\
	} while (0)

#define SCARD_DEBUG_ERR(fmt, ...)\
	do\
	{\
		LOGE(COLOR_RED fmt COLOR_END, ##__VA_ARGS__);\
	}while (0)

#define SCARD_BEGIN() \
	do\
    {\
		LOGD(COLOR_BLUE"BEGIN >>>>"COLOR_END);\
    } while( 0 )

#define SCARD_END() \
	do\
    {\
		LOGD(COLOR_BLUE"END <<<<"COLOR_END);\
    } \
    while( 0 )

#endif /* DEBUG_H_ */
