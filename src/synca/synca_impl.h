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

#include <atomic.h>
#include <common.h>
#include <log.h>
#include <stats.h>

#include <coro/coro.h>
#include <synca/synca.h>

#include "journey.h"
#include "state.h"

#define LOG_COMPONENT_SYNCA LOG_COMPONENT_MT << "[" << synca::detail::index() << "] "

namespace synca {

void waitForDone();

} // namespace synca
