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

#include <atomic.h>
#include <god_adapter/god_adapter.h>

#include "synca_fixture.h"

struct SlowCounter
{
    void inc()
    {
        Atomic<int> c;
        c.store(counter);
        SyncaFixture::sleepFast();
        counter = c + 1;
    }

    int get()
    {
        return counter;
    }

private:
    int counter = 0;
};

struct SlowCounterAtomic
{
    void inc()
    {
        SyncaFixture::sleepFast();
        counter += 1;
    }

    int get()
    {
        return counter;
    }

    int getImmediate()
    {
        return counter;
    }

private:
    Atomic<int> counter;
};

DECL_ADAPTER(SlowCounter, inc, get)
DECL_ADAPTER(SlowCounterAtomic, inc, get)
