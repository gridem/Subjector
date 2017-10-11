/*
 * Copyright 2017 Grigory Demchenko (aka gridem)
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

#include <god_adapter/decl.h>
#include <synca/synca.h>

#include "co_adapter.h"
#include "co_async.h"

namespace subjector {
namespace detail {

template <typename T_base>
struct BasePortal : T_base, private synca::SchedulerRef
{
    BasePortal() = default;

    template <typename... V>
    BasePortal(mt::IScheduler& scheduler, V&&... v)
        : T_base{std::forward<V>(v)...}
        , synca::SchedulerRef{scheduler}
    {
    }

protected:
    template <typename F, typename... V>
    auto call(F f, V&&... v)
    {
        synca::Portal _{scheduler()};
        return f(static_cast<T_base&>(*this), std::forward<V>(v)...);
    }

    using synca::SchedulerRef::scheduler;
};

template <typename T_base>
struct BaseSerializedPortal : T_base
{
    template <typename... V>
    BaseSerializedPortal(V&&... v, const char* name = "serialized")
        : T_base{std::forward<V>(v)...}
        , tp_(1, name)
    {
    }

protected:
    template <typename F, typename... V>
    auto call(F f, V&&... v)
    {
        synca::Portal _{tp_};
        return f(static_cast<T_base&>(*this), std::forward<V>(v)...);
    }

    auto& scheduler()
    {
        return tp_;
    }

private:
    mt::ThreadPool tp_;
};

} // namespace detail

template <typename T, typename T_async = CoEmpty, typename T_base = T>
using CoPortal = CoAdapter<T, typename T_async::template Go<detail::BasePortal<T_base>>>;

template <typename T, typename T_async = CoEmpty, typename T_base = T>
using CoSerializedPortal =
    CoAdapter<T, typename T_async::template Go<detail::BaseSerializedPortal<T_base>>>;

} // namespace subjector
