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
constexpr auto c_wait = "wait";
constexpr auto c_done = "done";
constexpr auto c_done2 = "done2";

BOOST_FIXTURE_TEST_SUITE(Wait, SyncaFixture)

BOOST_AUTO_TEST_CASE(WaitSimple)
{
    start("WaitSimple", [] {
        synca::Awaiter a;
        for (size_t i = 0; i < 5; ++i) {
            a.go([&, i] { op(c_done); });
        }
        a.wait();
        CHECK_OPS(c_done, c_done, c_done, c_done, c_done);
    });
}

BOOST_AUTO_TEST_CASE(WaitTrivial)
{
    start("WaitTrivial", [] {
        synca::Awaiter a;
        a.wait();
        a.wait();
    });
}

BOOST_AUTO_TEST_CASE(WaitCancel)
{
    start("WaitCancel", [] {
        synca::Awaiter a;
        BarrierCounter b;
        a.go([&] {
            op(c_go);
            b.wait(0);
            synca::checkEvents();
            op(c_done);
        });
        a.cancel();
        b.unblock();
        a.wait();
    });
    CHECK_OPS(c_go);
}

BOOST_AUTO_TEST_CASE(AwaitCancelDtor)
{
    BarrierCounter b;
    start("AwaitCancelDtor", [&] {
        auto g = synca::go([&] {
            synca::Awaiter a;
            a.go([&] {
                op(c_go);
                b.wait(0);
                sleep();
                synca::checkEvents();
                op(c_done);
            });
            a.wait();
        });
        g.cancel();
        b.unblock();
    });
    CHECK_OPS(c_go);
}

BOOST_AUTO_TEST_CASE(AwaitCancelGo)
{
    BarrierCounter b;
    start("AwaitCancelGo", [&] {
        auto g = synca::go([&] {
            synca::Awaiter a;
            a.go([&] {
                op(c_go);
                b.wait(1);
                sleep();
                synca::checkEvents();
                op(c_done2);
            });
            op(c_go);
            b.wait(0);
            op(c_wait);
            a.wait();
            op(c_done);
        });
        sleep();
        CHECK_OPS(c_go, c_go);
        b.unblock();
        sleep();
        CHECK_OPS(c_go, c_go, c_wait);
        g.cancel();
        b.unblock();
    });
    CHECK_OPS(c_go, c_go, c_wait);
}

BOOST_AUTO_TEST_SUITE_END()
