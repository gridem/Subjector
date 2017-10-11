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

#include "synca_impl.h"

#include <boost/asio.hpp>

namespace mt {
namespace detail {

struct Service : boost::asio::io_service
{
};

} // namespace detail
} // namespace mt

namespace synca {

struct Alone::Impl : boost::asio::io_service::strand
{
    using boost::asio::io_service::strand::strand;
};

Alone::Alone(mt::IScheduler& scheduler, const char* name)
    : impl_{std::make_unique<Impl>(mt::detail::schedulerToService(scheduler))}
    , name_(name)
{
}

Alone::~Alone()
{
}

void Alone::schedule(Handler handler)
{
    impl_->post(std::move(handler));
}

const char* Alone::name() const
{
    return name_;
}

} // namespace synca
