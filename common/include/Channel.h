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

#ifndef CHANNEL_H_
#define CHANNEL_H_

/* standard library header */

/* SLP library header */

/* local header */
#include "Synchronous.h"
#include "ByteArray.h"
#include "Exception.h"

namespace smartcard_service_api
{
	class SessionHelper;	/* explicit declaration */

	typedef void (*transmitCallback)(unsigned char *buffer, unsigned int length,
		int error, void *userParam);
	typedef void (*closeCallback)(int error, void *userParam);

	class Channel : public Synchronous
	{
	protected :
		ByteArray selectResponse;
		SessionHelper *session;
		int channelNum;

		Channel() : session(NULL), channelNum(-1) {}
		Channel(SessionHelper *session) : session(session), channelNum(-1) {}

		inline void setSelectResponse(ByteArray response) { selectResponse = response; }

	public :
		virtual ~Channel() {};

		inline bool isBasicChannel() const throw() { return (channelNum == 0); }
		inline bool isClosed() const throw() { return (channelNum < 0); }

		inline ByteArray getSelectResponse() const throw() { return selectResponse; }
		inline SessionHelper *getSession() const throw() { return session; }

		virtual int close(closeCallback callback, void *userParam) = 0;
		virtual int transmit(ByteArray command, transmitCallback callback, void *userData) = 0;

		virtual void closeSync()
			throw(ExceptionBase &, ErrorIO &, ErrorSecurity &,
			ErrorIllegalState &, ErrorIllegalParameter &) = 0;

		virtual int transmitSync(ByteArray command, ByteArray &result)
			throw(ExceptionBase &, ErrorIO &, ErrorIllegalState &,
				ErrorIllegalParameter &, ErrorSecurity &) = 0;
	};

} /* namespace smartcard_service_api */
#endif /* CHANNEL_H_ */
