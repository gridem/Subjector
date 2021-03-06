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

DECL_EXC(Error, std::runtime_error, "Synca error")
DECL_EXC(Event, Error, "Event received")
DECL_EXC(CancelledEvent, Event, "Cancel")
DECL_EXC(TimedoutEvent, Event, "Timeout")

struct DtorEventsGuard
{
    DtorEventsGuard();
    ~DtorEventsGuard() noexcept;

private:
    bool wasEnabled_;
};

void checkEvents();
bool disableEvents();
void disableEventsAndCheck();
void enableEvents();
void enableEventsAndCheck();

} // namespace synca
