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

#include "synca_impl.h"

#define LOG_COMPONENT_COMPLETER                                                          \
    LOG_COMPONENT_SYNCA << "completer " << jobs_ << ";"                                  \
                        << counter_.load(std::memory_order_relaxed) << ": "

namespace synca {

struct CompleterScope : DisableCopy
{
    CompleterScope(Completer& c)
        : c_{c}
    {
    }

    ~CompleterScope() noexcept
    {
        c_.done();
    }

private:
    Completer& c_;
};

Completer::Completer()
    : doer_{journey().doer()}
{
}

void Completer::start() noexcept
{
    ++jobs_;
}

void Completer::done() noexcept
{
    LOG(DEBUG, COMPLETER) << "completing";
    if (add(1)) {
        LOG(DEBUG, COMPLETER) << "done";
        doer_.done();
    }
}

void Completer::wait()
{
    if (jobs_ == 0) {
        LOG(DEBUG, COMPLETER) << "no need waiting";
        return;
    }
    wait0();
    jobs_ = 0;
}

bool Completer::add(int v) noexcept
{
    return counter_.fetch_add(v, std::memory_order_relaxed) + v == 0;
}

void Completer::wait0()
{
    if (jobs_ > 0) {
        jobs_ = -jobs_;
        if (add(jobs_)) {
            LOG(DEBUG, COMPLETER) << "done on first wait";
            return;
        }
        LOG(DEBUG, COMPLETER) << "first wait";
    } else {
        if (counter_.load(std::memory_order_relaxed) == 0) {
            LOG(DEBUG, COMPLETER) << "done on second wait";
            return;
        }
        LOG(DEBUG, COMPLETER) << "second wait";
    }
    LOG(DEBUG, COMPLETER) << "waiting";
    journey().waitForDone();
    LOG(DEBUG, COMPLETER) << "done after waiting";
}

Awaiter::~Awaiter() noexcept
{
    if (empty())
        return;
    DtorEventsGuard _;
    cancel();
    wait();
}

Awaiter& Awaiter::go(Handler h, mt::IScheduler& scheduler)
{
    goers_.emplace_back(synca::go(
        [ this, h = std::move(h) ] {
            CompleterScope _{completer_};
            h();
        },
        scheduler));
    completer_.start();
    return *this;
}

void Awaiter::wait()
{
    completer_.wait();
    goers_.clear();
}

void Awaiter::cancel() noexcept
{
    for (auto&& g : goers_)
        g.cancel();
}

bool Awaiter::empty() const noexcept
{
    return goers_.empty();
}

} // namespace synca
