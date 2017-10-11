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

void waitForAll();

struct Completer
{
    Completer();

    void start() noexcept;
    void done() noexcept; // threadsafe
    void wait();

private:
    bool add(int v) noexcept;
    void wait0();

    int jobs_ = 0;
    Atomic<int> counter_;
    Doer doer_;
};

struct Awaiter
{
    ~Awaiter() noexcept;
    Awaiter& go(Handler h, mt::IScheduler& scheduler = defaultScheduler());
    void wait();
    void cancel() noexcept;
    bool empty() const noexcept;

private:
    Completer completer_;
    std::vector<Goer> goers_;
};

} // namespace synca
