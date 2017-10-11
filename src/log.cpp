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

#include <iostream>
#include <mutex>

#include <boost/date_time/posix_time/posix_time.hpp>

#include "common.h"
#include "log.h"

namespace {

std::mutex& logMutex()
{
    return single<std::mutex, LogMessage>();
}

} // namespace

LogMessage::LogMessage()
    : out_{std::cerr}
{
    logMutex().lock();
    out_ << boost::posix_time::microsec_clock::local_time() << ": ";
}

LogMessage::~LogMessage()
{
    out_ << std::endl;
    logMutex().unlock();
}
