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

#include <god_adapter/shared.h>
#include <synca/spinlock.h>

#include "co_adapter.h"
#include "co_async.h"

namespace subjector {

template <typename T, typename T_async = CoEmpty, typename T_base = T>
using CoSpinlock = CoAdapter<
    T,
    typename T_async::template GoScheduler<BaseLocker<T_base, synca::Spinlock>>>;

} // namespace subjector
