/*
 * Copyright 2017 Grigory Demchenko (aka gridem)
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

#include "co_adapter.h"
#include "error.h"
#include "result.h"

#include "co_alone.h"
#include "co_async.h"
#include "co_channel.h"
#include "co_mutex.h"
#include "co_portal.h"
#include "co_spinlock.h"

namespace subjector {

#define BIND_SUBJECTOR(D_type, D_subjector, ...)                                         \
    template <>                                                                          \
    struct ::subjector::SubjectorPolicy<D_type>                                          \
    {                                                                                    \
        using Type = D_subjector<D_type, ##__VA_ARGS__>;                                 \
    };

template <typename T>
struct SubjectorPolicy
{
    using Type = CoMutex<T>;
};

template <typename T>
using Subjector = typename SubjectorPolicy<T>::Type;

} // namespace subjector
