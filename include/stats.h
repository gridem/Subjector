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

#pragma once

#include <atomic.h>
#include <common.h>

namespace synca {

using StatCounter = Atomic<uint64_t>;

namespace detail {

void registerStat(const char* name, StatCounter* v);
}

template <typename T>
struct Stat : StatCounter
{
    Stat()
    {
        detail::registerStat(typeid(T).name(), this);
    }
};

void dumpStats();

template <typename T>
StatCounter& stat()
{
    return single<Stat<T>, T>();
}

template <typename T>
uint64_t addStat(int64_t delta)
{
    return stat<T>().fetch_add(delta, std::memory_order_relaxed) + delta;
}

template <typename T>
uint64_t incStat()
{
    return addStat<T>(1);
}

template <typename T>
uint64_t decStat()
{
    return addStat<T>(-1);
}

template <typename T>
struct InstanceStat
{
    struct InstanceCreated
    {
    };

    struct InstanceDestroyed
    {
    };

    InstanceStat()
    {
        incStat<InstanceCreated>();
    }

    ~InstanceStat()
    {
        incStat<InstanceDestroyed>();
    }

    static uint64_t createCounter()
    {
        return stat<InstanceCreated>().load(std::memory_order_relaxed);
    }

    static uint64_t destroyCounter()
    {
        return stat<InstanceDestroyed>().load(std::memory_order_relaxed);
    }
};
}
