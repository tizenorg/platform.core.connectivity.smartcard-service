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


#ifndef SESSIONHELPER_H_
#define SESSIONHELPER_H_

/* standard library header */
#include <vector>

/* SLP library header */

/* local header */
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

	class SessionHelper : public Synchronous
	{
	protected:
		ReaderHelper *reader;
		vector<Channel *> channels;
		ByteArray atr;
		bool closed;

	public:
		SessionHelper(ReaderHelper *reader);
		virtual ~SessionHelper() {}

		ReaderHelper *getReader() { return reader; }
		bool isClosed() { return closed; }

		virtual void closeChannels() = 0;

		virtual int getATR(getATRCallback callback, void *userData) = 0;
		virtual int close(closeSessionCallback callback, void *userData) = 0;

		virtual int openBasicChannel(ByteArray aid, openChannelCallback callback, void *userData) = 0;
		virtual int openBasicChannel(unsigned char *aid, unsigned int length, openChannelCallback callback, void *userData) = 0;
		virtual int openLogicalChannel(ByteArray aid, openChannelCallback callback, void *userData) = 0;
		virtual int openLogicalChannel(unsigned char *aid, unsigned int length, openChannelCallback callback, void *userData) = 0;

		virtual ByteArray getATRSync() = 0;
		virtual void closeSync() = 0;

		virtual Channel *openBasicChannelSync(ByteArray aid) = 0;
		virtual Channel *openBasicChannelSync(unsigned char *aid, unsigned int length) = 0;
		virtual Channel *openLogicalChannelSync(ByteArray aid) = 0;
		virtual Channel *openLogicalChannelSync(unsigned char *aid, unsigned int length) = 0;
	};

} /* namespace smartcard_service_api */
#endif /* SESSIONHELPER_H_ */
