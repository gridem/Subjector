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
    LOG_COMPONENT_SYNCA << "completer, jobs: " << jobs_ << ", "

#define LOG_COMPONENT_EVENTS LOG_COMPONENT_SYNCA << "events: "

namespace synca {

DtorEventsGuard::DtorEventsGuard()
    : wasEnabled_{journey().disableEvents()}
{
    LOG(DEBUG, EVENTS) << "dtor created";
}

DtorEventsGuard::~DtorEventsGuard() noexcept
{
    LOG(DEBUG, EVENTS) << "dtor destroyed, was enabled: " << wasEnabled_;
    if (wasEnabled_)
        journey().enableEvents();
}

void checkEvents()
{
    journey().checkEvents();
}

} // namespace synca
