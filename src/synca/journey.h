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

#include "synca_impl.h"

namespace synca {

struct Journey : InstanceStat<Journey>
{
    void proceed();
    Handler doneHandler();
    void teleport(mt::IScheduler& s);
    void waitForDone();
    void reschedule();

    void handleEvents();
    void checkEvents();
    bool disableEvents();
    void enableEvents();
    void enableEventsAndCheck();
    void detachDoers();

    mt::IScheduler& scheduler() const;
    int index() const;
    Goer goer() const;
    Doer doer() const;
    DetachableDoer detachableDoer() const;

    static Goer start(Handler handler, mt::IScheduler& s);

private:
    Journey(Handler handler, mt::IScheduler& s);
    ~Journey()
    {
    }

    Handler proceedHandler();
    Handler coroHandler(Handler);
    Goer start0();
    void schedule0(Handler);
    void proceed0();
    void suspend();

    coro::Coro coro_;
    mt::IScheduler* scheduler_ = nullptr;
    std::shared_ptr<State> state_;
};

Journey& journey();

} // namespace synca
