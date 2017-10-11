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

#include <coro/coro.h>
#include <log.h>
#include <mt/mt.h>

#define LOG_COMPONENT_CORO LOG_COMPONENT_MT << "{" << stateToString0() << "} "

namespace coro {

namespace {

using namespace boost::context::detail;
constexpr size_t c_stackSize = 1024 * 32; // 32K ought to be enough for anybody

} // namespace

Coro::Coro(Handler handler)
    : stack_(c_stackSize)
    , handler_{std::move(handler)}
{
    coroContext_ = boost::context::detail::make_fcontext(
        &stack_.back(), stack_.size(), [](boost::context::detail::transfer_t p) {
            static_cast<Coro*>(p.data)->starter0(p.fctx);
        });
    LOG(DEBUG, CORO) << "coro context created";
}

Coro::~Coro() noexcept
{
    if (!isCompleted())
        LOG(ERROR, CORO) << "destroying noncompleted coro";
    LOG(DEBUG, CORO) << "coro destroyed";
}

// continue coroutine execution after yield
void Coro::resume()
{
    LOG(DEBUG, CORO) << "resuming";
    VERIFY(state_ == State::Suspended, "Coro must be suspended");
    resume0(static_cast<void*>(this));
}

// yields execution to external context
void Coro::yield()
{
    LOG(DEBUG, CORO) << "yielding";
    VERIFY(state_ == State::Running, "Coro must be running");
    state_ = State::Suspended;
    yield0();
    state_ = State::Running;
    LOG(DEBUG, CORO) << "yielded";
}

// is coroutine was started and not completed
bool Coro::isCompleted() const noexcept
{
    return state_ == State::Completed;
}

// returns to external context
void Coro::yield0() noexcept
{
    externalContext_ = jump_fcontext(externalContext_, 0).fctx;
}

void Coro::resume0(void* p) noexcept
{
    coroContext_ = jump_fcontext(coroContext_, p).fctx;
}

void Coro::starter0(Coro::Ctx ctx) noexcept
{
    externalContext_ = ctx;
    state_ = State::Running;
    /*
     * If exception is thrown inside handler then
     * we don't have a consistent state
     * but usually after that the application is crashed
     * so it makes no sense to do it using RAII
     */
    handler_();
    state_ = State::Completed;
    yield0();
}

const char* Coro::stateToString0() const
{
    switch (state_) {
    case State::Suspended:
        return "suspended";

    case State::Running:
        return "running";

    case State::Completed:
        return "completed";

    default:
        return "<unknown>";
    }
}

} // namespace coro
