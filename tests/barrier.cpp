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

#include <mt/mt.h>

#include "barrier.h"

Barrier::Barrier(bool blocked)
    : blocked_{blocked}
{
}

void Barrier::block(bool blocked)
{
    blocked_.store(blocked);
}

void Barrier::unblock()
{
    block(false);
}

void Barrier::wait() const
{
    mt::waitFor([this] { return blocked_.load(); });
}

void BarrierCounter::wait(int toWait) const
{
    mt::waitFor([this, toWait] { return counter_ > toWait; });
}

void BarrierCounter::unblock()
{
    ++counter_;
}
