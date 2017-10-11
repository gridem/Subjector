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

namespace synca {

Doer::Doer()
    : Doer{journey().doer()}
{
}

Doer::Doer(State& state)
    : state_{state}
{
}

void Doer::done()
{
    state_.done();
}

void Doer::cancel()
{
    state_.cancel();
}

void Doer::wait()
{
    journey().waitForDone();
}

Goer::Goer()
{
}

Goer::Goer(const std::shared_ptr<State>& state)
    : state_{state}
{
}

void Goer::cancel()
{
    if (auto s = state_.lock())
        s->cancel();
}

void Goer::timedout()
{
    if (auto s = state_.lock())
        s->timedout();
}

DetachableDoer::DetachableDoer()
    : DetachableDoer{journey().detachableDoer()}
{
}

DetachableDoer::DetachableDoer(const std::shared_ptr<State>& state)
    : state_{state}
    , counter_{state->counter()}
    , eventsEnabled_{state->isEventsEnabled()}
{
}

bool DetachableDoer::acquire()
{
    return state_->acquire(counter_, eventsEnabled_);
}

void DetachableDoer::releaseAndDone()
{
    state_->releaseAndDone(eventsEnabled_);
}

void DetachableDoer::done()
{
    if (acquire())
        releaseAndDone();
}

void DetachableDoer::wait()
{
    journey().waitForDone();
}

} // namespace synca
