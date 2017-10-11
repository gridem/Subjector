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

#include <tls.h>

#define LOG_COMPONENT_JR LOG_COMPONENT_SYNCA

namespace synca {
namespace detail {

int index()
{
    Journey* j = tlsPtr<Journey>();
    return j ? j->index() : 0;
}

} // namespace detal

struct JourneyStat
{
    struct DoneWithoutRescheduling
    {
    };
    struct DoneWithRescheduling
    {
    };
    struct ReleaseOnDisabledEvents
    {
    };
    struct Suspended
    {
    };
    struct CoroYield
    {
    };
    struct TeleportPerformed
    {
    };
    struct TeleportSkipped
    {
    };
};

Journey::Journey(Handler handler, mt::IScheduler& s)
    : coro_{coroHandler(std::move(handler))}
    , scheduler_{&s}
    , state_{std::make_shared<State>(*this)}
{
}

Handler Journey::coroHandler(Handler handler)
{
    return [ this, handler = std::move(handler) ]
    {
        LOG(DEBUG, JR) << "journey started";
        try {
            /*
             * It's unsafe to invoke handleEvents before handler
             * The reason is that the user may want to use the RAII object
             * at the beginning of the handler.
             * And it's not obvious the it must be placed inside handler closure
             * as a variables
             * So the user should use reschedule to cancel the coroutine
             */
            handler();
        } catch (std::exception& e) {
            LOG(ERROR, JR) << "exception in journey: " << e.what();
        }
        LOG(DEBUG, JR) << "journey ended";
    };
}

Handler Journey::doneHandler()
{
    Doer d = doer();
    return [d]() mutable { d.done(); };
}

void Journey::proceed()
{
    schedule0([this] { proceed0(); });
}

Handler Journey::proceedHandler()
{
    return [this] { proceed(); };
}

void Journey::teleport(mt::IScheduler& s)
{
    if (&s == scheduler_) {
        incStat<JourneyStat::TeleportSkipped>();
        LOG(DEBUG, JR) << "the same destination, skipping teleport <-> " << s.name();
        return;
    }
    incStat<JourneyStat::TeleportPerformed>();
    LOG(DEBUG, JR) << "teleport " << scheduler_->name() << " -> " << s.name();
    scheduler_ = &s;
    reschedule();
}

void Journey::waitForDone()
{
    if (state_->isDone()) {
        incStat<JourneyStat::DoneWithoutRescheduling>();
        LOG(DEBUG, JR) << "the work was done, continuing without rescheduling";
    } else {
        incStat<JourneyStat::DoneWithRescheduling>();
        suspend();
    }
    handleEvents();
}

void Journey::reschedule()
{
    state_->done();
    suspend();
    handleEvents();
}

void Journey::handleEvents()
{
    state_->handleEvents();
}

void Journey::checkEvents()
{
    state_->checkEvents();
}

bool Journey::disableEvents()
{
    return state_->disableEvents();
}

void Journey::enableEvents()
{
    state_->enableEvents();
}

void Journey::enableEventsAndCheck()
{
    state_->enableEventsAndCheck();
}

void Journey::detachDoers()
{
    state_->detachDoers();
}

mt::IScheduler& Journey::scheduler() const
{
    return *scheduler_;
}

int Journey::index() const
{
    return state_->index();
}

Goer Journey::goer() const
{
    return Goer{state_};
}

Doer Journey::doer() const
{
    return Doer{*state_};
}

DetachableDoer Journey::detachableDoer() const
{
    return DetachableDoer{state_};
}

Goer Journey::start(Handler handler, mt::IScheduler& s)
{
    /*
     * Two-phase init due to:
     * 1. exception safety guarantee
     * 2. goer must be returned before starting to avoid races
     */
    return (new Journey(std::move(handler), s))->start0();
}

Goer Journey::start0()
{
    Goer g = goer();
    proceed();
    return g;
}

void Journey::schedule0(Handler handler)
{
    VERIFY(scheduler_ != nullptr, "Scheduler must be set in journey");
    scheduler_->schedule(std::move(handler));
}

void Journey::proceed0()
{
    {
        auto _ = tlsGuard(this);
        coro_.resume();
        if (coro_.isCompleted()) {
            delete this;
            return;
        }
    }
    state_->resetEnteredAndCheckEvents();
}

void Journey::suspend()
{
    LOG(DEBUG, JR) << "journey suspending";
    incStat<JourneyStat::Suspended>();
    incStat<JourneyStat::CoroYield>();
    coro_.yield();
    decStat<JourneyStat::Suspended>();
    LOG(DEBUG, JR) << "journey resumed";
}

Journey& journey()
{
    return tls<Journey>();
}

void waitForAll()
{
    LOG(DEBUG, MT) << "waiting for journeys to complete";
    mt::waitFor([] { return Journey::createCounter() == Journey::destroyCounter(); });
    LOG(DEBUG, MT) << "waiting for journeys completed";
}

} // namespace synca
