/*
 * Copyright 2015 Grigory Demchenko (aka gridem)
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

#include "counter.h"
#include "ut.h"

BOOST_FIXTURE_TEST_SUITE(Portal, OpsFixture)

namespace {

thread_local int value = 0;

struct Val
{
    void checkFor(int val)
    {
        BOOST_TEST(value == val);
    }
};

struct Val2 : Val
{
};

} // namespace

BOOST_AUTO_TEST_CASE(Teleport)
{
    mt::ThreadPool tp1(1, "tp1");
    mt::ThreadPool tp2(1, "tp2");

    BarrierCounter b;
    tp1.schedule([&] {
        value = 1;
        b.unblock();
    });
    tp2.schedule([&] {
        value = 2;
        b.unblock();
    });
    b.wait(1);
    BOOST_TEST(value == 0);
    synca::go(
        [&] {
            BOOST_TEST(value == 1);
            synca::teleport(tp2);
            BOOST_TEST(value == 2);
            synca::teleport(tp1);
            BOOST_TEST(value == 1);
            b.unblock();
        },
        tp1);
    b.wait(2);
}

BOOST_AUTO_TEST_CASE(PortalSimple)
{
    mt::ThreadPool tp1(1, "tp1");
    mt::ThreadPool tp2(1, "tp2");

    BarrierCounter b;
    tp1.schedule([&] {
        value = 1;
        b.unblock();
    });
    tp2.schedule([&] {
        value = 2;
        b.unblock();
    });
    b.wait(1);
    BOOST_TEST(value == 0);
    synca::go(
        [&] {
            BOOST_TEST(value == 1);
            {
                synca::Portal _(tp2);
                BOOST_TEST(value == 2);
            }
            BOOST_TEST(value == 1);
            b.unblock();
        },
        tp1);
    b.wait(2);
}

BOOST_AUTO_TEST_CASE(Portal)
{
    mt::ThreadPool tp1(1, "tp1");
    mt::ThreadPool tp2(1, "tp2");
    mt::ThreadPool tp3(1, "tp3");

    synca::portal<Val>().attach(tp2);
    synca::portal<Val2>().attach(tp3);

    BarrierCounter b;
    tp1.schedule([&] {
        value = 1;
        b.unblock();
    });
    tp2.schedule([&] {
        value = 2;
        b.unblock();
    });
    tp3.schedule([&] {
        value = 3;
        b.unblock();
    });
    b.wait(2);
    BOOST_TEST(value == 0);
    synca::go(
        [&] {
            BOOST_TEST(value == 1);
            synca::portal<Val>()->checkFor(2);
            BOOST_TEST(value == 1);
            synca::portal<Val2>()->checkFor(3);
            BOOST_TEST(value == 1);
            synca::teleport(tp3);
            BOOST_TEST(value == 3);
            synca::portal<Val>()->checkFor(2);
            BOOST_TEST(value == 3);
            synca::teleport(tp2);
            BOOST_TEST(value == 2);
            synca::portal<Val>()->checkFor(2);
            BOOST_TEST(value == 2);
            b.unblock();
        },
        tp1);
    b.wait(3);
}

BOOST_AUTO_TEST_CASE(Alone)
{
    synca::DefaultThreadPool tp{4, "Alone"};
    BarrierCounter b;
    BarrierCounter bMain;
    synca::Alone alone(tp);
    synca::portal<SlowCounter>().attach(alone);
    for (int i = 0; i < 4; ++i)
        synca::go([&] {
            bMain.unblock();
            b.wait(0);
            for (int i = 0; i < 100; ++i) {
                synca::portal<SlowCounter>()->inc();
            }
        });
    bMain.wait(3);
    b.unblock();
    synca::waitForAll();
    BOOST_TEST(single<SlowCounter>().get() == 100 * 4);
}

BOOST_AUTO_TEST_SUITE_END()
