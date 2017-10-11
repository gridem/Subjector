/*
 * Copyright 2015-2017 Grigory Demchenko (aka gridem)
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

#include <coro/coro.h>

#include "ut.h"

constexpr auto c_handler = "handler";

BOOST_FIXTURE_TEST_SUITE(Coro, OpsFixture)

BOOST_AUTO_TEST_CASE(Create)
{
    coro::Coro coro([] { op(c_handler); });
    CHECK_OPS();
    // test generates log: {suspended} destroying noncompleted coro
}

BOOST_AUTO_TEST_CASE(ResumeSimple)
{
    coro::Coro coro([] { op(c_handler); });
    CHECK_OPS();
    coro.resume();
    BOOST_CHECK(coro.isCompleted());
    CHECK_OPS(c_handler);
}

BOOST_AUTO_TEST_CASE(ResumeVerificationError)
{
    coro::Coro coro([] { op(c_handler); });
    CHECK_OPS();
    coro.resume();
    BOOST_CHECK(coro.isCompleted());
    CHECK_OPS(c_handler);
    BOOST_REQUIRE_THROW(coro.resume(), VerificationError);
    CHECK_OPS(c_handler);
}

BOOST_AUTO_TEST_CASE(Yield)
{
    coro::Coro coro([&coro] {
        BOOST_CHECK(!coro.isCompleted());
        op(c_handler);
        coro.yield();
        BOOST_CHECK(!coro.isCompleted());
        op(c_handler);
    });
    CHECK_OPS();
    coro.resume();
    BOOST_CHECK(!coro.isCompleted());
    CHECK_OPS(c_handler);
    coro.resume();
    BOOST_CHECK(coro.isCompleted());
    CHECK_OPS(c_handler, c_handler);
}

BOOST_AUTO_TEST_SUITE_END()
