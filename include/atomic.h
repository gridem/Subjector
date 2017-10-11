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

#include <atomic>

#include "common.h"

// atomic with zero value initialization
template <typename T>
struct Atomic : std::atomic<T>
{
    Atomic(T v = T())
        : std::atomic<T>(v)
    {
    }
};

template <typename T, typename T_tag>
std::atomic<T>& atomic()
{
    return single<Atomic<T>, T_tag>();
}

template <typename T_tag>
std::atomic<int>& atomicInt()
{
    return atomic<int, T_tag>();
}
