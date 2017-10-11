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

namespace synca {

namespace {

auto& defaultSchedulerPtr()
{
    return single<mt::IScheduler*>();
}

} // namespace

mt::IScheduler& defaultScheduler()
{
    auto scheduler = defaultSchedulerPtr();
    VERIFY(scheduler != nullptr, "Default scheduler is not attached");
    return *scheduler;
}

void attachDefaultScheduler(mt::IScheduler& scheduler)
{
    defaultSchedulerPtr() = &scheduler;
}

SchedulerRef::SchedulerRef()
    : scheduler_{defaultSchedulerPtr()}
{
}

SchedulerRef::SchedulerRef(mt::IScheduler& s)
    : scheduler_{&s}
{
}

void SchedulerRef::attach(mt::IScheduler& s)
{
    scheduler_ = &s;
}

void SchedulerRef::detach()
{
    scheduler_ = nullptr;
}

mt::IScheduler& SchedulerRef::scheduler() const
{
    VERIFY(scheduler_ != nullptr, "Scheduler is not attached");
    return *scheduler_;
}

SchedulerRef::operator mt::IScheduler&() const
{
    return *scheduler_;
}

void reschedule()
{
    journey().reschedule();
}

DefaultThreadPool::DefaultThreadPool(size_t threadCount, const char* name)
    : mt::ThreadPool{threadCount, name}
{
    attachDefaultScheduler(*this);
}

} // namespace synca
