/*
 * Copyright 2014-2017 Grigory Demchenko (aka gridem)
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

#include <condition_variable>
#include <mutex>
#include <thread>

#include <boost/asio.hpp>
#include <boost/optional.hpp>

#include <common.h>
#include <log.h>

#include <mt/mt.h>

namespace mt {

using Lock = std::unique_lock<std::mutex>;

namespace detail {

std::thread createThread(Handler handler, int number, const char* name);

} // namespace detail
} // namespace mt
