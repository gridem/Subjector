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

#include <boost/context/detail/fcontext.hpp>

namespace coro {

// coroutine class
struct Coro
{
    // Create coroutine without starting
    // need to execute resume function to start it.
    // User is responsible to provide exception-safe handler
    explicit Coro(Handler);

    ~Coro() noexcept;

    // continue coroutine execution after yield
    void resume();

    // is coroutine execution completed
    bool isCompleted() const noexcept;

    // return back from coroutine
    void yield();

private:
    using Ctx = boost::context::detail::fcontext_t;

    void yield0() noexcept;
    void resume0(void* p = 0) noexcept;
    void starter0(Ctx ctx) noexcept;
    const char* stateToString0() const;

    enum class State
    {
        Suspended,
        Running,
        Completed,
    };

    State state_ = State::Suspended;

    Ctx coroContext_;
    Ctx externalContext_;
    std::vector<uint8_t> stack_;
    Handler handler_;
};

} // namespace coro
