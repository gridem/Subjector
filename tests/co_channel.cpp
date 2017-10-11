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

#include "co_cases.h"

namespace subjector {

template <typename T, typename T_async = void, typename T_base = T>
using CoChannelCompat = CoChannel<T, T_base>;

} // namespace subjector

BOOST_FIXTURE_TEST_SUITE(CoChannel, SyncaFixture)

BOOST_AUTO_TEST_CASE(Result)
{
    subjector::Result<int> v;
    v.set(2);
    BOOST_TEST(v.get() == 2);
}

BOOST_AUTO_TEST_CASE(CoSimple)
{
    start("CoSimple", [&] {
        subjector::CoChannel<SlowCounter> counter;
        auto v = counter.get();
        BOOST_TEST(v == 0);
    });
}

BOOST_AUTO_TEST_CASE(CoSetGet)
{
    start("CoSimple", [&] {
        subjector::CoChannel<SlowCounter> counter;
        counter.inc();
        auto v = counter.get();
        BOOST_TEST(v == 1);
    });
}

BOOST_AUTO_TEST_CASE(CoLoop)
{
    static constexpr int amount = 100;
    mt::ThreadPool tp{4, "Common"};
    start("CoSimple", [&] {
        subjector::CoChannel<SlowCounter> counter(tp);
        for (int i = 0; i < amount; ++i) {
            counter.inc();
        }
        auto v = counter.get();
        BOOST_TEST(v == amount);
    });
}

SUBJECTOR_GO_CASES(CoChannelCompat)

BOOST_AUTO_TEST_SUITE_END()
