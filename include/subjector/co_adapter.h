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

namespace subjector {
namespace detail {

template <typename T_derived>
struct BaseAsyncWrapper
{
protected:
    template <typename... V>
    auto call(V&&... v)
    {
        return static_cast<T_derived&>(*this).asyncCall(std::forward<V>(v)...);
    }
};

template <typename T, typename T_base>
struct BaseCoAdapter : T_base,
                       private Adapter<T, BaseAsyncWrapper<BaseCoAdapter<T, T_base>>>
{
    FWD_CTOR_TBASE(BaseCoAdapter)

    friend class BaseAsyncWrapper<BaseCoAdapter>;

    auto& async()
    {
        return static_cast<Adapter<T, BaseAsyncWrapper<BaseCoAdapter>>&>(*this);
    }

protected:
    using T_base::call;
};

} // namespace detail

template <typename T, typename T_base = T>
using CoAdapter = Adapter<T, detail::BaseCoAdapter<T, T_base>>;

} // namespace subjector
