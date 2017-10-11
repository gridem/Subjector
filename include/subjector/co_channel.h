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

#include <boost/variant.hpp>

#include "co_adapter.h"
#include "error.h"
#include "result.h"

namespace subjector {
namespace detail {

template <typename T_base>
struct BaseChannel : T_base, synca::SchedulerRef
{
    BaseChannel()
    {
        start();
    }

    template <typename... V>
    BaseChannel(mt::IScheduler& scheduler, V&&... v)
        : T_base{std::forward<V>(v)...}
        , synca::SchedulerRef{scheduler}
    {
        start();
    }

    ~BaseChannel()
    {
        stop();
    }

    void stop()
    {
        channel_.close();
        // need to wait because channel must be alive during closing for-loop
        awaiter_.wait();
    }

    void wait()
    {
        if (channel_.empty())
            return;
        synca::DetachableDoer doer;
        channel_.put([doer]() mutable { doer.done(); });
        doer.wait();
    }

protected:
    template <typename F, typename... V>
    auto call(F f, V&&... v)
    {
        auto fun = [&] { return f(static_cast<T_base&>(*this), std::forward<V>(v)...); };
        WrappedResult<decltype(fun())> result;
        synca::DetachableDoer doer;
        channel_.put([&, doer]() mutable {
            if (!doer.acquire())
                return;
            try {
                result.set(wrap(fun));
            } catch (std::exception&) {
                result.setCurrentError();
            }
            doer.releaseAndDone();
        });
        doer.wait();
        return result.get().unwrap();
    }

    template <typename F, typename... V>
    auto asyncCall(F f, V&&... v)
    {
        channel_.put([ f = std::move(f), v..., this ] {
            try {
                f(static_cast<T_base&>(*this), std::move(v)...);
            } catch (std::exception&) {
                // do nothing due to async call
            }
        });
    }

private:
    void start()
    {
        awaiter_.go([&] { loop(); }, synca::SchedulerRef::scheduler());
    }

    void loop()
    {
        for (auto&& action : channel_) {
            action();
        }
    }

    synca::Channel<Handler> channel_;
    synca::Awaiter awaiter_;
};

} // namespace detail

template <typename T, typename T_base = T>
using CoChannel = CoAdapter<T, detail::BaseChannel<T_base>>;

} // namespace subjector
