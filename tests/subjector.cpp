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

#include "counter.h"
#include "ut.h"

#include <subjector/subjector.h>
#include <synca/synca.h>

namespace ns {

struct SlowCounterSpinlock : SlowCounter
{
};

} // namespace ns

DECL_ADAPTER(ns::SlowCounterSpinlock, inc, get)
BIND_SUBJECTOR(ns::SlowCounterSpinlock, CoChannel)

BOOST_FIXTURE_TEST_SUITE(Subjector, SyncaFixture)

BOOST_AUTO_TEST_CASE(Simple)
{
    start("Simple", [&] {
        subjector::Subjector<SlowCounter> counter;
        auto v = counter.get();
        BOOST_TEST(v == 0);
    });
}

BOOST_AUTO_TEST_CASE(SetGet)
{
    start("CoSimple", [&] {
        subjector::Subjector<SlowCounter> counter;
        counter.inc();
        auto v = counter.get();
        BOOST_TEST(v == 1);
    });
}

BOOST_AUTO_TEST_CASE(SetGetSpinlock)
{
    start("CoSimple", [&] {
        subjector::Subjector<ns::SlowCounterSpinlock> counter;
        counter.inc();
        auto v = counter.get();
        BOOST_TEST(v == 1);
    });
}

BOOST_AUTO_TEST_SUITE_END()
