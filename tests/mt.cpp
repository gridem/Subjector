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

#include <atomic.h>
#include <mt/mt.h>

#include "ut.h"

BOOST_FIXTURE_TEST_SUITE(MT, OpsFixture)

BOOST_AUTO_TEST_CASE(Empty)
{
    mt::ThreadPool tp(4, "tp");
    tp.wait();
}

BOOST_AUTO_TEST_CASE(EmptyWait)
{
    mt::ThreadPool tp(4, "tp");
    tp.wait();
    tp.wait();
    tp.wait();
}

BOOST_AUTO_TEST_CASE(CounterWait)
{
    mt::ThreadPool tp(4, "tp");
    Atomic<int> v;
    tp.wait();
    BOOST_CHECK_EQUAL(v, 0);
    tp.schedule([&] { ++v; });
    tp.wait();
    BOOST_CHECK_EQUAL(v, 1);
}

BOOST_AUTO_TEST_CASE(Counter100)
{
    mt::ThreadPool tp(4, "tp");
    Atomic<int> v;
    for (int i = 0; i < 100; ++i)
        tp.schedule([&] { ++v; });
    tp.wait();
    BOOST_CHECK_EQUAL(v, 100);
}

BOOST_AUTO_TEST_SUITE_END()
