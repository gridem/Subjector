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

#include <ostream>

#include "common.h"

struct LogMessage
{
    LogMessage();
    ~LogMessage();

    template <typename T>
    LogMessage& operator<<(T&& t)
    {
        out_ << std::forward<T>(t);
        return *this;
    }

private:
    std::ostream& out_;
};

#define LOG_MESSAGE                                                                      \
    LogMessage                                                                           \
    {                                                                                    \
    }

#define LOG(D_level, D_component)                                                        \
    if (LOG_LEVEL_##D_level)                                                             \
    LOG_MESSAGE << LOG_COMPONENT_##D_component

#if defined(flagLOG_DEBUG)
#define LOG_LEVEL_DEBUG true
#else
#define LOG_LEVEL_DEBUG false
#endif

#define LOG_LEVEL_INFO true
#define LOG_LEVEL_ERROR true
