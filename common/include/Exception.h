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

#ifndef EXCEPTION_H_
#define EXCEPTION_H_

#include <exception>
#include <stddef.h>

#include "smartcard-types.h"

namespace smartcard_service_api
{
	class ExceptionBase : public std::exception
	{
	protected :
		int errorCode;

	public :
		ExceptionBase(int errorCode) throw()
			: errorCode(errorCode) {}
		virtual ~ExceptionBase() throw() {}

		int getErrorCode() throw() { return errorCode; }
		virtual const char *what() const throw()
		{
			return "Unknown exception";
		}
	};

	class ErrorIO : public ExceptionBase
	{
	private :
		unsigned char sw[2];

	public :
		ErrorIO(int errorCode) throw()
			: ExceptionBase(errorCode) {}
		ErrorIO(int errorCode, unsigned char *sw) throw()
			: ExceptionBase(errorCode)
		{
			if (sw != NULL)
			{
				this->sw[0] = sw[0];
				this->sw[1] = sw[1];
			}
		}
		virtual ~ErrorIO() throw() {}

		unsigned short getSW() throw()
			{ return (unsigned short)(sw[0] << 8 | sw[1]); }
		unsigned char getSW1() throw() { return sw[0]; }
		unsigned char getSW2() throw() { return sw[1]; }

		virtual const char *what() const throw()
		{
			const char *result = NULL;

			switch (errorCode)
			{
			case SCARD_ERROR_IPC_FAILED :
				result = "Failed to communicate with server";
				break;

			case SCARD_ERROR_IO_FAILED :
				result = "IO Operation failed";
				break;

			default :
				result = ExceptionBase::what();
				break;
			}

			return result;
		}
	};

	class ErrorSecurity : public ExceptionBase
	{
	public :
		ErrorSecurity(int errorCode) throw()
			: ExceptionBase(errorCode) {}
		virtual ~ErrorSecurity() throw() {}

		virtual const char *what() const throw()
		{
			const char *result = NULL;

			switch (errorCode)
			{
			case SCARD_ERROR_SECURITY_NOT_ALLOWED :
				result = "Access denied";
				break;

			default :
				result = ExceptionBase::what();
				break;
			}

			return result;
		}
	};

	class ErrorIllegalState : public ExceptionBase
	{
	public :
		ErrorIllegalState(int errorCode) throw()
			: ExceptionBase(errorCode) {}
		virtual ~ErrorIllegalState() throw() {}

		virtual const char *what() const throw()
		{
			const char *result = NULL;

			switch (errorCode)
			{
			case SCARD_ERROR_UNAVAILABLE :
				result = "Closed instance";
				break;

			case SCARD_ERROR_NOT_INITIALIZED :
				result = "Need to initialize IPC";
				break;

			case SCARD_ERROR_SE_NOT_INITIALIZED :
				result = "Need to initialize SE";
				break;

			default :
				result = ExceptionBase::what();
				break;
			}

			return result;
		}
	};

	class ErrorIllegalParameter : public ExceptionBase
	{
	public :
		ErrorIllegalParameter(int errorCode) throw()
			: ExceptionBase(errorCode) {}
		virtual ~ErrorIllegalParameter() throw() {}

		virtual const char *what() const throw()
		{
			const char *result = NULL;

			switch (errorCode)
			{
			case SCARD_ERROR_ILLEGAL_PARAM :
				result = "Incorrect format of parameter";
				break;

			default :
				result = ExceptionBase::what();
				break;
			}

			return result;
		}
	};
} /* namespace smartcard_service_api */

#endif /* EXCEPTIONBASE_H_ */
