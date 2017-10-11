/*
 * Copyright 2014-2017 Grigory Demchenko (aka gridem)
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

#include "mt_impl.h"

namespace mt {

namespace {

thread_local int t_number = 0;
thread_local const char* t_name = "main";

} // namespace

namespace detail {

const char* name()
{
    return t_name;
}

int number()
{
    return t_number;
}

std::thread createThread(Handler handler, int number, const char* name)
{
    return std::thread([ h = std::move(handler), number, name ] {
        t_number = number + 1;
        t_name = name;
        try {
            LOG(DEBUG, MT) << "thread created";
            h();
            LOG(DEBUG, MT) << "thread ended";
        } catch (std::exception& e) {
            LOG(DEBUG, MT) << "thread ended with error: " << e.what();
        }
    });
}

Service& schedulerToService(IScheduler& scheduler)
{
    IService* service = dynamic_cast<IService*>(&scheduler);
    VERIFY(service != nullptr, "Scheduler must be implemented using thread pool");
    return service->service();
}

} // namespace detail

void sleepFor(Duration duration)
{
    std::this_thread::sleep_for(duration);
}

void yield()
{
    std::this_thread::yield();
}

int concurrency()
{
    return std::thread::hardware_concurrency();
}

void waitFor(Predicate condition)
{
    while (!condition())
        yield();
}

} // namespace mt
