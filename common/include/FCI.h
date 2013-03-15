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

#ifndef FCI_H_
#define FCI_H_

/* standard library header */

/* SLP library header */

/* local header */
#include "ByteArray.h"

namespace smartcard_service_api
{
	class FCP
	{
	private:
		ByteArray fcpBuffer;

		char strBuffer[400];

		unsigned int fileSize;
		unsigned int totalFileSize;
		unsigned int fid;
		unsigned int sfi;
		unsigned int maxRecordSize;
		unsigned int numberOfRecord;
		unsigned int fileType;
		unsigned int fileStructure;
		unsigned int lcs;

		void resetMemberVar();

	public:
		FCP();
		FCP(ByteArray &array);
		~FCP();

		bool setFCP(ByteArray array);
		ByteArray getFCP();
		void releaseFCP();

		unsigned int getFileSize();
		unsigned int getTotalFileSize();
		unsigned int getFID();
		unsigned int getSFI();
		unsigned int getMaxRecordSize();
		unsigned int getNumberOfRecord();
		unsigned int getFileType();
		unsigned int getFileStructure();
		unsigned int getLCS();

		const char *toString();
	};

	class FCM
	{
	private:
		ByteArray fcmBuffer;

	public:
		FCM();
		virtual ~FCM();
	};

	class FCI
	{
	private:
		ByteArray fciBuffer;
		FCP fcp;
		FCM fcm;

	public:
		static const int INFO_NOT_AVAILABLE = -1;

		static const int FT_DF = 0;
		static const int FT_EF = 1;

		static const int FS_NO_EF = 0;
		static const int FS_TRANSPARENT = 1;
		static const int FS_LINEAR_FIXED = 2;
		static const int FS_LINEAR_VARIABLE = 3;
		static const int FS_CYCLIC = 4;

		static const int LCS_NO_INFORMATION_GIVEN = 0;
		static const int LCS_CREATION_STATE = 1;
		static const int LCS_INITIALISATION_STATE = 3;
		static const int LCS_OPERATION_STATE_ACTIVATED = 5;
		static const int LCS_OPERATION_STATE_DEACTIVATED = 4;
		static const int LCS_TERMINATION_STATE = 6;

		FCI();
		~FCI();

		bool setFCIBuffer(ByteArray array);
	};

} /* namespace smartcard_service_api */
#endif /* FCI_H_ */
