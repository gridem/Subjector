/*
 * Copyright 2015-2017 Grigory Demchenko (aka gridem)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "synca_impl.h"

#define LOG_COMPONENT_MUTEX LOG_COMPONENT_SYNCA << "mutex [" << index_ << "]: "

namespace synca {

struct MutexStat
{
    struct Lock
    {
    };
    struct LockNoWait
    {
    };
    struct LockWait
    {
    };
    struct Unlock
    {
    };
    struct UnlockAcquired
    {
    };
    struct UnlockNotAcquired
    {
    };
    struct UnlockNoWaiters
    {
    };
};

struct Mutex::Impl
{
    using Lock = std::unique_lock<std::mutex>;

    void lock()
    {
        incStat<MutexStat::Lock>();
        LOG(DEBUG, MUTEX) << "lock";
        {
            Lock _{mutex_};
            if (!acquired_) {
                LOG(DEBUG, MUTEX) << "acquired";
                acquired_ = true;
                incStat<MutexStat::LockNoWait>();
                return;
            }
            LOG(DEBUG, MUTEX) << "wait";
            incStat<MutexStat::LockWait>();
            doers_.push(DetachableDoer{});
        }
        journey().waitForDone();
        LOG(DEBUG, MUTEX) << "wait done";
    }

    void unlock()
    {
        incStat<MutexStat::Unlock>();
        LOG(DEBUG, MUTEX) << "unlock";
        Lock _{mutex_};
        while (!doers_.empty()) {
            LOG(DEBUG, MUTEX) << "unlock nonempty queue";
            DetachableDoer d = doers_.pop();
            if (d.acquire()) {
                _.unlock();
                LOG(DEBUG, MUTEX) << "unlock acquired";
                incStat<MutexStat::UnlockAcquired>();
                d.releaseAndDone();
                return;
            }
            LOG(DEBUG, MUTEX) << "unlock not acquired";
            incStat<MutexStat::UnlockNotAcquired>();
        }
        LOG(DEBUG, MUTEX) << "unlock no waiters";
        incStat<MutexStat::UnlockNoWaiters>();
        acquired_ = false;
    }

    int index_ = ++atomicInt<struct Index>();
    mutable std::mutex mutex_;
    bool acquired_ = false;
    Queue<DetachableDoer> doers_;
};

Mutex::Mutex()
    : impl_{std::make_unique<Impl>()}
{
}

Mutex::~Mutex()
{
}

void Mutex::lock()
{
    impl_->lock();
}

void Mutex::unlock()
{
    impl_->unlock();
}

} // namespace synca
