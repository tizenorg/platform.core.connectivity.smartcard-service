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

#ifndef LOCK_H_
#define LOCK_H_

#include <pthread.h>

#include "Debug.h"

namespace smartcard_service_api
{
	class EXPORT Lock
	{
	public:
		virtual ~Lock() {};
		virtual void lock() = 0;
		virtual void unlock() = 0;
	};

	class PMutex : public Lock
	{
	private:
		pthread_mutex_t mutex;

	public:
		PMutex() { pthread_mutex_init(&mutex, NULL); }
		~PMutex() { pthread_mutex_destroy(&mutex); }

		inline void lock() { pthread_mutex_lock(&mutex); }
		inline void unlock() { pthread_mutex_unlock(&mutex); }
	};
#define TOKENPASTE(x, y) x ## y
#define TOKENPASTE2(x, y) TOKENPASTE(x, y)
#define SCOPE_LOCK(X) \
	if (const AutoLockHelper& TOKENPASTE2(lock_, __LINE__) = makeAutoLock(X))

	class EXPORT AutoLockHelper
	{
	public:
		inline operator bool() const
		{
			return true;
		}
	};

	template<typename T>
	class EXPORT AutoLock : public AutoLockHelper
	{
	private:
		T *lock;

	public:
		AutoLock(const AutoLock &_lock) { lock = _lock.lock; }
		AutoLock(T& _lock) : lock(&_lock) { lock->lock(); }
		~AutoLock() { lock->unlock(); }
	};

	template<typename T>
	inline AutoLock<T> makeAutoLock(T& lock)
	{
		return AutoLock<T>(lock);
	}

} /* namespace smartcard_service_api */
#endif /* SCOPELOCK_H_ */
