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

#include <synca/synca.h>

#include "ut.h"

constexpr auto c_go = "go";

BOOST_FIXTURE_TEST_SUITE(Synca, OpsFixture)

BOOST_AUTO_TEST_CASE(Go)
{
    synca::DefaultThreadPool tp{1, "Go"};
    synca::go([] { op(c_go); });
    synca::waitForAll();
    CHECK_OPS(c_go);
}

BOOST_AUTO_TEST_CASE(GoBarrier)
{
    synca::DefaultThreadPool tp{1, "GoBarrier"};
    BarrierCounter bGo;
    BarrierCounter bMain;
    synca::go([&] {
        bGo.wait(0);
        op(c_go);
        bMain.unblock();
        bGo.wait(1);
    });
    CHECK_OPS();
    bGo.unblock();
    bMain.wait(0);
    CHECK_OPS(c_go);
    bGo.unblock();
    synca::waitForAll();
    CHECK_OPS(c_go);
}

BOOST_AUTO_TEST_CASE(GoerEmpty)
{
    synca::Goer g;
    g.cancel();
    g.timedout();
}

BOOST_AUTO_TEST_SUITE_END()
