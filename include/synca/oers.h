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

struct State;

struct Doer
{
    Doer();
    explicit Doer(State& state);

    void done();
    void cancel();
    void wait();

private:
    State& state_;
};

struct Goer
{
    Goer();
    explicit Goer(const std::shared_ptr<State>& state);

    void cancel();
    void timedout();

private:
    std::weak_ptr<State> state_;
};

struct DetachableDoer
{
    DetachableDoer();
    explicit DetachableDoer(const std::shared_ptr<State>& state);

    bool acquire();
    void releaseAndDone();
    void done(); // done only if acquired();
    void wait();

private:
    std::shared_ptr<State> state_;
    int counter_;
    bool eventsEnabled_;
};

} // namespace synca
