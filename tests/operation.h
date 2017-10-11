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

#include <common.h>
#include <mutex>
#include <vector>

#define CHECK_OPS(...) BOOST_CHECK_EQUAL(ops(), (Operations{__VA_ARGS__}));

using Operations = std::vector<const char*>;

struct OperationsData
{
    Operations operations() const
    {
        std::lock_guard<std::mutex> _{mutex_};
        return ops_;
    }

    void append(const char* op)
    {
        std::lock_guard<std::mutex> _{mutex_};
        ops_.push_back(op);
    }

    void clear()
    {
        std::lock_guard<std::mutex> _{mutex_};
        ops_.clear();
    }

private:
    Operations ops_;
    mutable std::mutex mutex_;
};

inline Operations ops()
{
    return single<OperationsData>().operations();
}

inline void op(const char* o)
{
    single<OperationsData>().append(o);
}

struct OpsFixture
{
    ~OpsFixture()
    {
        single<OperationsData>().clear();
    }
};
