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

namespace subjector {
namespace detail {

template <typename T_base>
struct BaseEmpty : T_base
{
    FWD_CTOR_TBASE(BaseEmpty)
};

struct GoBase
{
    template <typename... V>
    auto go(V&&... v)
    {
        return synca::go(std::forward<V>(v)...);
    }
};

struct AwaitGoBase
{
    void wait()
    {
        awaiter_.wait();
    }

protected:
    template <typename... V>
    void go(V&&... v)
    {
        awaiter_.go(std::forward<V>(v)...);
    }

private:
    synca::Awaiter awaiter_;
};

template <typename T_goBase>
struct CoAsyncBase
{
    template <typename T_base>
    struct Go : T_base, T_goBase
    {
        FWD_CTOR_TBASE(Go)

    protected:
        template <typename F, typename... V>
        auto asyncCall(F&& f, V&&... v)
        {
            return T_goBase::go(
                [ f = std::move(f), v..., this ]() mutable {
                    f(static_cast<T_base&>(*this), std::move(v)...);
                },
                T_base::scheduler());
        }
    };

    template <typename T_base>
    struct GoScheduler : T_base, synca::SchedulerRef, T_goBase
    {
        GoScheduler() = default;

        template <typename... V>
        GoScheduler(mt::IScheduler& scheduler, V&&... v)
            : T_base{std::forward<V>(v)...}
            , synca::SchedulerRef::SchedulerRef{scheduler}
        {
        }

    protected:
        template <typename... V>
        auto asyncCall(V&&... v)
        {
            return T_goBase::go(
                [v..., this]() mutable { T_base::call(std::move(v)...); },
                synca::SchedulerRef::scheduler());
        }
    };
};

} // namespace detail

/*
 * CoEmpty avoids unnecessary creating of async implementation.
 */
struct CoEmpty
{
    template <typename T_base>
    using Go = detail::BaseEmpty<T_base>;

    template <typename T_base>
    using GoScheduler = detail::BaseEmpty<T_base>;
};

/*
 * CoAsync policy is used to execute asynchronous operation using `sync::go`.
 */
using CoAsync = detail::CoAsyncBase<detail::GoBase>;

/*
 * CoAsyncAwait policy is used to execute asynchronous operation using `Awaiter::go`.
 *
 * It's preferred policy to use in asynchronous code because it allows to wait all
 * on-the-fly coroutines automatically.
 */
using CoAsyncAwait = detail::CoAsyncBase<detail::AwaitGoBase>;

} // namespace subjector
