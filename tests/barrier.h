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

#include <atomic.h>

struct Barrier
{
    explicit Barrier(bool blocked = true);

    void block(bool blocked = true);
    void unblock();
    void wait() const;

private:
    Atomic<bool> blocked_;
};

struct BarrierCounter
{
    void wait(int toWait) const;
    void unblock();

private:
    Atomic<int> counter_;
};

template <typename T>
struct Unblocker
{
    Unblocker(T& t)
        : t_{t}
    {
    }

    ~Unblocker()
    {
        t_.unblock();
    }

private:
    T& t_;
};
