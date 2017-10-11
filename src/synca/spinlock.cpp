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

#include <synca/spinlock.h>

#include "synca_impl.h"

namespace synca {

void Spinlock::lock()
{
    while (lock_.test_and_set(std::memory_order_acquire)) {
        reschedule();
    }
}

void Spinlock::unlock()
{
    lock_.clear(std::memory_order_release);
}

} // namespace synca
