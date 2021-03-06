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

#pragma once

#include "synca.h"

namespace synca {

mt::IScheduler& defaultScheduler();
void attachDefaultScheduler(mt::IScheduler& scheduler);

struct SchedulerRef
{
    SchedulerRef();
    explicit SchedulerRef(mt::IScheduler& s);

    void attach(mt::IScheduler& s);
    void detach();
    mt::IScheduler& scheduler() const;
    operator mt::IScheduler&() const;

private:
    mt::IScheduler* scheduler_ = nullptr;
};

void reschedule();

struct DefaultThreadPool : mt::ThreadPool
{
    explicit DefaultThreadPool(size_t threadCount, const char* name = "");
};

} // namespace synca
