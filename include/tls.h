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

#include "common.h"

template <typename T, typename T_tag = T>
T*& tlsPtr()
{
    static thread_local T* t = nullptr;
    return t;
}

template <typename T, typename T_tag = T>
T& tls()
{
    T* t = tlsPtr<T, T_tag>();
    VERIFY(t != nullptr, "TLS must be initialized");
    return *t;
}

template <typename T>
struct TlsGuard : DisableCopy
{
    TlsGuard(T* that)
    {
        auto& p = tlsPtr<T>();
        that_ = p;
        p = that;
    }

    ~TlsGuard()
    {
        tlsPtr<T>() = that_;
    }

private:
    T* that_;
};

// here I rely on RVO optimization to avoid unnecesary checkings and implementation
// complexity
template <typename T>
TlsGuard<T> tlsGuard(T* that)
{
    return that;
}
