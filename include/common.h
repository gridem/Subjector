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

#include <functional>
#include <memory>
#include <queue>
#include <string>
#include <vector>

#define VERIFY(D_cond, D_str)                                                            \
    if (!(D_cond))                                                                       \
    throw VerificationError(#D_cond ": " D_str)

#define DECL_EXC(D_name, D_base, D_msg)                                                  \
    struct D_name : D_base                                                               \
    {                                                                                    \
        D_name()                                                                         \
            : D_base{D_msg}                                                              \
        {                                                                                \
        }                                                                                \
        D_name(const std::string& msg)                                                   \
            : D_base{D_msg ": " + msg}                                                   \
        {                                                                                \
        }                                                                                \
    };

DECL_EXC(VerificationError, std::runtime_error, "Verification failed")

using Byte = unsigned char;
using Ptr = Byte*;
using PtrArray = Ptr*;
using IntArray = int*;
using Buffer = std::vector<Byte>;
using Handler = std::function<void()>;
using Predicate = std::function<bool()>;
using Handlers = std::initializer_list<Handler>;

struct IObject
{
    virtual ~IObject()
    {
    }
};

template <typename T, typename T_tag = T>
T& single()
{
    static T t;
    return t;
}

template <typename T, typename T_tag = T>
const T& singleConst()
{
    return single<T, T_tag>();
}

/*
 * Class to avoid unnecessary copying for guards.
 * =delete for copy ctor results to the compilation error.
 * So you should rely on RVO optimization here.
 * To verify RVO the exception will be thrown.
 */
struct DisableCopy
{
    DisableCopy() = default;

    DisableCopy(const DisableCopy&)
    {
        forbidden();
    }

    DisableCopy& operator=(const DisableCopy&) = delete;

private:
    void forbidden()
    {
        throw std::logic_error("Forbidden: looks like RVO optimization goes away");
    }
};

template <typename T>
struct Shared : std::shared_ptr<T>
{
    Shared()
        : std::shared_ptr<T>(std::make_shared<T>())
    {
    }
};

template <typename T>
struct Queue
{
    bool empty() const
    {
        return q_.empty();
    }

    void clear()
    {
        q_.clear();
    }

    T pop()
    {
        T t{std::move(q_.front())};
        q_.pop();
        return t;
    }

    void push(T t)
    {
        q_.emplace(std::move(t));
    }

    void swap(Queue& q)
    {
        q_.swap(q.q_);
    }

private:
    std::queue<T> q_;
};

template <typename T>
struct Wrap
{
    template <typename F>
    Wrap(F&& f)
        : value_{f()}
    {
    }

    T&& unwrap()
    {
        return std::move(value_);
    }

    const T& unwrap() const
    {
        return value_;
    }

private:
    T value_;
};

template <>
struct Wrap<void>
{
    template <typename F>
    Wrap(F&& f)
    {
        f();
    }

    void unwrap() const
    {
    }
};

template <typename F>
auto wrap(F&& f)
{
    return Wrap<decltype(f())>(std::forward<F>(f));
}
