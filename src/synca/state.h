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

struct Journey;

struct State
{
    explicit State(Journey& j);

    enum
    {
        FlagEntered = 1 << 0,
        FlagEventsEnabled = 1 << 1,
        FlagDone = 1 << 2,
        FlagCancelled = 1 << 3, // raise event
        FlagTimedout = 1 << 4,  // raise event
        ValueCounter = 1 << 5,
        MaskCounter = ~(ValueCounter - 1), // the rest is the counter
    };

    void cancel();
    void timedout();
    // returns: was events enabled?
    bool disableEvents();
    void enableEvents();
    void enableEventsAndCheck();
    bool isDone() const;
    bool isEventsEnabled();
    void done();
    void handleEvents();
    void checkEvents();
    void resetEnteredAndCheckEvents();
    bool acquire(int oldCounter, bool wasEventsEnabled);
    void releaseAndDone(bool wasEventsEnabled);
    void detachDoers();
    int counter();
    int index() const;

private:
    void handleRaiseEvents(int eventFlag);
    void addRaiseEvent(int eventFlag);
    int resetFlags0(int flags);
    int setFlags0(int flags);
    void proceed();
    static bool mayProceed(int state);
    static std::string stateToString(int state);
    static std::string statesToString(int oldState, int state);

    // FlagDone MUST be used only if handleEvents before handler
    Atomic<int> state_{FlagEntered | FlagEventsEnabled /* | FlagDone*/};
    Journey& j_;
    int index_; // to avoid races when we try to send an event and put logs
};

} // namespace synca
