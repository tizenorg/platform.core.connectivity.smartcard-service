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

#ifndef SESSIONHELPER_H_
#define SESSIONHELPER_H_

#include <vector>

#include "Debug.h"
#include "Synchronous.h"
#include "ByteArray.h"
#include "Channel.h"

using namespace std;

namespace smartcard_service_api
{
	class ReaderHelper;

	typedef void (*openChannelCallback)(Channel *channel, int error, void *userData);
	typedef void (*getATRCallback)(unsigned char *atr, unsigned int length, int error, void *userData);
	typedef void (*closeSessionCallback)(int error, void *userData);
	typedef void (*getChannelCountCallback)(unsigned count, int error, void *userData);

	class EXPORT SessionHelper : public Synchronous
	{
	protected:
		ReaderHelper *reader;
		vector<Channel *> channels;
		ByteArray atr;
		bool closed;

	public:
		SessionHelper(ReaderHelper *reader) :
			reader(reader), closed(true) {}
		virtual ~SessionHelper() {}

		ReaderHelper *getReader() const throw() { return reader; }
		bool isClosed() const throw() { return closed; }

		virtual void closeChannels()
			throw(ErrorIO &, ErrorIllegalState &) = 0;

		virtual int getATR(getATRCallback callback, void *userData) = 0;
		virtual int close(closeSessionCallback callback, void *userData) = 0;

		virtual int openBasicChannel(const ByteArray &aid, openChannelCallback callback, void *userData) = 0;
		virtual int openBasicChannel(const unsigned char *aid, unsigned int length, openChannelCallback callback, void *userData) = 0;
		virtual int openLogicalChannel(const ByteArray &aid, openChannelCallback callback, void *userData) = 0;
		virtual int openLogicalChannel(const unsigned char *aid, unsigned int length, openChannelCallback callback, void *userData) = 0;

		virtual const ByteArray getATRSync()
			throw(ExceptionBase &, ErrorIO &, ErrorSecurity &,
			ErrorIllegalState &, ErrorIllegalParameter &) = 0;

		virtual void closeSync()
			throw(ExceptionBase &, ErrorIO &, ErrorSecurity &,
			ErrorIllegalState &, ErrorIllegalParameter &) = 0;

		virtual Channel *openBasicChannelSync(const ByteArray &aid)
			throw(ExceptionBase &, ErrorIO &, ErrorIllegalState &,
				ErrorIllegalParameter &, ErrorSecurity &) = 0;

		virtual Channel *openBasicChannelSync(const unsigned char *aid, unsigned int length)
			throw(ExceptionBase &, ErrorIO &, ErrorIllegalState &,
				ErrorIllegalParameter &, ErrorSecurity &) = 0;

		virtual Channel *openLogicalChannelSync(const ByteArray &aid)
			throw(ExceptionBase &, ErrorIO &, ErrorIllegalState &,
				ErrorIllegalParameter &, ErrorSecurity &) = 0;

		virtual Channel *openLogicalChannelSync(const unsigned char *aid, unsigned int length)
			throw(ExceptionBase &, ErrorIO &, ErrorIllegalState &,
				ErrorIllegalParameter &, ErrorSecurity &) = 0;
	};

} /* namespace smartcard_service_api */
#endif /* SESSIONHELPER_H_ */
