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

#include <boost/variant.hpp>

#include <common.h>

#include "error.h"

namespace subjector {

template <typename T>
struct Result
{
    bool empty() const
    {
        return boost::get<Void>(&result_) != nullptr;
    }

    bool isResult() const
    {
        return boost::get<T>(&result_) != nullptr;
    }

    bool isError() const
    {
        return boost::get<std::exception_ptr>(&result_) != nullptr;
    }

    const T& get() const
    {
        return boost::apply_visitor(Extractor{}, result_);
    }

    void setCurrentError()
    {
        result_ = std::current_exception();
    }

    template <typename U>
    void set(U&& u)
    {
        result_ = std::forward<U>(u);
    }

private:
    struct Void
    {
    };

    struct Extractor : public boost::static_visitor<const T&>
    {
        const T& operator()(Void) const
        {
            throw EmptyResult();
        }

        const T& operator()(const std::exception_ptr& ex) const
        {
            std::rethrow_exception(ex);
        }

        const T& operator()(const T& val) const
        {
            return val;
        }
    };

    boost::variant<Void, std::exception_ptr, T> result_;
};

template <typename T>
using WrappedResult = Result<Wrap<T>>;

} // namespace subjector
