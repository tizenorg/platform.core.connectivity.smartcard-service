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

#include <string>

#include "ByteArray.h"

namespace smartcard_service_api
{
	class FCP
	{
	private:
		ByteArray fcpBuffer;

		int fileSize;
		int totalFileSize;
		int fid;
		int sfi;
		int maxRecordSize;
		int numberOfRecord;
		int fileType;
		int fileStructure;
		int lcs;

		void resetMemberVar();

	public:
		FCP();
		FCP(const ByteArray &array);
		~FCP();

		bool setFCP(const ByteArray &array);
		const ByteArray getFCP() const;
		void releaseFCP();

		int getFileSize() const;
		int getTotalFileSize() const;
		int getFID() const;
		int getSFI() const;
		int getMaxRecordSize() const;
		int getNumberOfRecord() const;
		int getFileType() const;
		int getFileStructure() const;
		int getLCS() const;

		const string toString() const;
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

		bool setFCIBuffer(const ByteArray &array);
	};

} /* namespace smartcard_service_api */
#endif /* FCI_H_ */
