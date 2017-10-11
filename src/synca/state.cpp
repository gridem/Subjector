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

#define LOG_COMPONENT_STATE LOG_COMPONENT_SYNCA << "-> [" << index_ << "]: "

namespace synca {

struct StateStat
{
    struct SendRaiseEvent
    {
    };
    struct RaiseCancelEvent
    {
    };
    struct RaiseTimeoutEvent
    {
    };
    struct ProceedOnExit
    {
    };
    struct ProceedOnDone
    {
    };
    struct ProceedOnRaiseEvent
    {
    };
    struct ProceedOnReleaseDone
    {
    };
    struct AcquiredOnDisabledEvents
    {
    };
    struct AcquiringFailed
    {
    };
    struct Acquired
    {
    };
    struct ReleaseOnDisabledEvents
    {
    };
};

State::State(synca::Journey& j)
    : j_{j}
    , index_{++atomicInt<struct Index>()}
{
}

void State::cancel()
{
    addRaiseEvent(FlagCancelled);
}

void State::timedout()
{
    addRaiseEvent(FlagTimedout);
}

bool State::disableEvents()
{
    return (resetFlags0(FlagEventsEnabled) & FlagEventsEnabled) != 0;
}

void State::enableEvents()
{
    setFlags0(FlagEventsEnabled);
}

void State::enableEventsAndCheck()
{
    handleRaiseEvents(setFlags0(FlagEventsEnabled));
}

bool State::isDone() const
{
    int state = state_.load(std::memory_order_relaxed);
    return (state & FlagDone) != 0;
}

void State::done()
{
    int old = setFlags0(FlagDone | FlagEntered);
    if ((old & FlagEntered) == 0) {
        incStat<StateStat::ProceedOnDone>();
        LOG(DEBUG, STATE) << "proceed on done: " << stateToString(old);
        proceed();
    }
}

void State::handleEvents()
{
    int event = state_.load(std::memory_order_relaxed);
    if (event & FlagEventsEnabled) {
        VERIFY(
            (event & (FlagCancelled | FlagTimedout | FlagDone)) != 0, "Must have event");
        handleRaiseEvents(event);
        // goes to FlagDone resetting on exception absence
    } else {
        VERIFY((event & FlagDone) != 0, "Must be done");
    }
    resetFlags0(FlagDone);
}

void State::checkEvents()
{
    int event = state_.load(std::memory_order_relaxed);
    LOG(DEBUG, STATE) << "check events: " << stateToString(event);
    if (event & FlagEventsEnabled)
        handleRaiseEvents(event);
}

void State::resetEnteredAndCheckEvents()
{
    int prev = state_.load(std::memory_order_relaxed);
    do {
        if (mayProceed(prev)) {
            incStat<StateStat::ProceedOnExit>();
            LOG(DEBUG, STATE) << "proceed in resetEnteredAndCheckEvents: "
                              << stateToString(prev);
            proceed();
            return;
        }
    } while (!state_.compare_exchange_weak(
        prev, prev & ~FlagEntered, std::memory_order_relaxed));
    LOG(DEBUG, STATE) << "reset entered, was: " << stateToString(prev);
}

bool State::acquire(int oldCounter, bool wasEventsEnabled)
{
    if (!wasEventsEnabled) {
        // we don't need to check the counter because
        // the only case when we can proceed is by proceed with the only done
        incStat<StateStat::AcquiredOnDisabledEvents>();
        LOG(DEBUG, STATE) << "events was disabled => acquired counter: " << oldCounter;
        return true;
    }
    int prev = state_.load(std::memory_order_relaxed);
    do {
        int counter = prev & MaskCounter;
        if (oldCounter != counter) {
            incStat<StateStat::AcquiringFailed>();
            LOG(DEBUG, STATE) << "event occured, cannot acquire: expected counter: "
                              << oldCounter << ", " << stateToString(prev);
            return false;
        }
    } while (!state_.compare_exchange_weak(
        prev, prev & ~FlagEventsEnabled, std::memory_order_relaxed));
    incStat<StateStat::Acquired>();
    LOG(DEBUG, STATE) << "reset entered, was: " << stateToString(prev);
    return true;
}

void State::releaseAndDone(bool wasEventsEnabled)
{
    if (!wasEventsEnabled) {
        incStat<StateStat::ReleaseOnDisabledEvents>();
        done();
        return;
    }
    int old = setFlags0(FlagDone | FlagEntered | FlagEventsEnabled);
    if ((old & FlagEntered) == 0) {
        incStat<StateStat::ProceedOnReleaseDone>();
        LOG(DEBUG, STATE) << "proceed on done with events enabled: "
                          << stateToString(old);
        proceed();
    } else {
        LOG(DEBUG, STATE) << "release and done, wait to enter: " << stateToString(old);
    }
}

void State::detachDoers()
{
    state_.fetch_add(ValueCounter, std::memory_order_relaxed);
    int state = resetFlags0(FlagDone);
    LOG(DEBUG, STATE) << "detached doers: " << stateToString(state);
}

int State::counter()
{
    return state_.load(std::memory_order_relaxed) & MaskCounter;
}

int State::index() const
{
    return index_;
}

bool State::isEventsEnabled()
{
    return (state_.load(std::memory_order_relaxed) & FlagEventsEnabled) != 0;
}

void State::handleRaiseEvents(int eventFlag)
{
    if (eventFlag & FlagCancelled) {
        // cancel has more priority and resets timeout event
        resetFlags0(FlagCancelled | FlagTimedout | FlagDone);
        incStat<StateStat::RaiseCancelEvent>();
        LOG(DEBUG, STATE) << "throwing event: cancel";
        throw CancelledEvent();
    }
    if (eventFlag & FlagTimedout) {
        resetFlags0(FlagTimedout | FlagDone);
        incStat<StateStat::RaiseTimeoutEvent>();
        LOG(DEBUG, STATE) << "throwing event: timeout";
        throw TimedoutEvent();
    }
}

void State::addRaiseEvent(int eventFlag)
{
    incStat<StateStat::SendRaiseEvent>();
    int prev = state_.load(std::memory_order_relaxed);
    LOG(DEBUG, STATE) << "adding raise event: " << statesToString(prev, eventFlag);
    while (true) {
        int next = prev | eventFlag;
        if ((prev & FlagEventsEnabled) != 0) {
            if (state_.compare_exchange_weak(
                    prev,
                    (next + ValueCounter) | FlagEntered,
                    std::memory_order_relaxed)) {
                if ((next & FlagEntered) == 0) {
                    incStat<StateStat::ProceedOnRaiseEvent>();
                    LOG(DEBUG, STATE) << "proceed on raise event: "
                                      << statesToString(prev, next);
                    proceed();
                } else {
                    LOG(DEBUG, STATE) << "cannot proceed on raise event: entered: "
                                      << statesToString(prev, next);
                }
                return;
            }
        } else {
            if (state_.compare_exchange_weak(prev, next, std::memory_order_relaxed)) {
                LOG(DEBUG, STATE) << "cannot proceed on raise event: disabled: "
                                  << statesToString(prev, next);
                return;
            }
        }
    }
}

int State::resetFlags0(int flags)
{
    int old = state_.fetch_and(~flags, std::memory_order_relaxed);
    LOG(DEBUG, STATE) << "reset flags: " << statesToString(old, flags);
    return old;
}

int State::setFlags0(int flags)
{
    int old = state_.fetch_or(flags, std::memory_order_relaxed);
    LOG(DEBUG, STATE) << "set flags: " << statesToString(old, flags);
    return old;
}

void State::proceed()
{
    j_.proceed();
}

bool State::mayProceed(int state)
{
    // either done or has allowed raise events (cancel or timedout)
    return (state & FlagDone) != 0 || ((state & FlagEventsEnabled) != 0 &&
                                       (state & (FlagCancelled | FlagTimedout)) != 0);
}

std::string State::stateToString(int state)
{
    std::string res = std::to_string((state & MaskCounter) / ValueCounter);
    auto add = [&](int flag, const char* name) {
        if ((flag & state) == 0)
            return;
        res += "|";
        res += name;
    };
    add(FlagEntered, "entered");
    add(FlagEventsEnabled, "enabled");
    add(FlagDone, "done");
    add(FlagCancelled, "cancelled");
    add(FlagTimedout, "timedout");
    return res;
}

std::string State::statesToString(int oldState, int state)
{
    return stateToString(state) + " <: " + stateToString(oldState);
}

} // namespace synca
