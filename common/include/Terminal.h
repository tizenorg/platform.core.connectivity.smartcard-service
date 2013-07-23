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

#ifndef TERMINAL_H_
#define TERMINAL_H_

/* standard library header */

/* SLP library header */

/* local header */
#include "ByteArray.h"
#include "Synchronous.h"

namespace smartcard_service_api
{
	typedef void (*terminalNotificationCallback)(void *terminal, int event, int error, void *user_param);

	typedef void (*terminalTransmitCallback)(unsigned char *buffer, unsigned int length, int error, void *userParam);
	typedef void (*terminalGetATRCallback)(unsigned char *buffer, unsigned int length, int error, void *userParam);

	class Terminal : public Synchronous
	{
	protected:
		terminalNotificationCallback statusCallback;
		bool initialized;
		char *name;

	public:
		static const int NOTIFY_SE_AVAILABLE = 1;
		static const int NOTIFY_SE_NOT_AVAILABLE = -1;

		Terminal() : statusCallback(NULL),
			initialized(false), name(NULL) {}

		virtual ~Terminal() {}

		virtual bool initialize() = 0;
		virtual void finalize() = 0;
		inline bool isInitialized() { return initialized; }

		inline char *getName() { return name; }
		inline void setStatusCallback(terminalNotificationCallback callback) { statusCallback = callback; }

		virtual bool isSecureElementPresence() = 0;

		virtual int transmitSync(ByteArray command, ByteArray &result) = 0;
		virtual int getATRSync(ByteArray &atr) = 0;

		virtual int transmit(ByteArray command, terminalTransmitCallback callback, void *userData) = 0;
		virtual int getATR(terminalGetATRCallback callback, void *userData) = 0;
	};

} /* namespace smartcard_service_api */
#endif /* TERMINAL_H_ */
