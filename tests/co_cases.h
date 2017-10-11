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

#include "barrier.h"
#include "counter.h"
#include "ut.h"

#include <subjector/subjector.h>
#include <synca/synca.h>

template <template <typename...> typename T_co>
void testCo()
{
    SyncaFixture::start("testCo", [] {
        BarrierCounter b;
        BarrierCounter bMain;
        T_co<SlowCounter> counter;
        synca::Awaiter a;
        for (int i = 0; i < 4; ++i)
            a.go([&] {
                bMain.unblock();
                b.wait(0);
                for (int i = 0; i < 100; ++i) {
                    counter.inc();
                }
            });
        bMain.wait(3);
        b.unblock();

        a.wait();

        int counterValue = 0;
        a.go([&] { counterValue = counter.get(); }).wait();
        BOOST_TEST(counterValue == 100 * 4);
    });
}

template <template <typename...> typename T_co>
void testCoScheduler()
{
    mt::ThreadPool tp{6, "testCoScheduler"};
    synca::go(
        [&] {
            BarrierCounter b;
            BarrierCounter bMain;
            T_co<SlowCounter> counter{tp};
            synca::Awaiter a;
            for (int i = 0; i < 4; ++i)
                a.go(
                    [&] {
                        bMain.unblock();
                        b.wait(0);
                        for (int i = 0; i < 100; ++i) {
                            counter.inc();
                        }
                    },
                    tp);
            bMain.wait(3);
            b.unblock();

            a.wait();

            int counterValue = 0;
            a.go([&] { counterValue = counter.get(); }, tp).wait();
            BOOST_TEST(counterValue == 100 * 4);
        },
        tp);
    synca::waitForAll();
}

template <template <typename...> typename T_co>
void testCoAsync()
{
    synca::DefaultThreadPool tp{6, "testCoAsyncScheduler"};
    BarrierCounter b;
    BarrierCounter bMain;
    T_co<SlowCounter, subjector::CoAsync> counter;
    for (int i = 0; i < 4; ++i)
        synca::go([&] {
            bMain.unblock();
            b.wait(0);
            for (int i = 0; i < 100; ++i) {
                counter.async().inc();
            }
        });
    bMain.wait(3);
    b.unblock();
    synca::waitForAll();

    int counterValue = 0;
    synca::go([&] { counterValue = counter.get(); });
    synca::waitForAll();
    BOOST_TEST(counterValue == 100 * 4);
}

template <template <typename...> typename T_co>
void testCoAsyncScheduler()
{
    mt::ThreadPool tp{6, "testCoAsyncScheduler"};
    BarrierCounter b;
    BarrierCounter bMain;
    T_co<SlowCounter, subjector::CoAsync> counter{tp};
    for (int i = 0; i < 4; ++i)
        synca::go(
            [&] {
                bMain.unblock();
                b.wait(0);
                for (int i = 0; i < 100; ++i) {
                    counter.async().inc();
                }
            },
            tp);
    bMain.wait(3);
    b.unblock();
    synca::waitForAll();

    int counterValue = 0;
    synca::go([&] { counterValue = counter.get(); }, tp);
    synca::waitForAll();
    BOOST_TEST(counterValue == 100 * 4);
}

template <template <typename...> typename T_co>
void testCoAsyncParallel()
{
    synca::DefaultThreadPool tp{4, "testCoAsyncParallel"};
    T_co<SlowCounterAtomic, subjector::CoAsync> counter;

    for (int i = 0; i < 4; ++i)
        counter.async().inc();

    BOOST_TEST(counter.getImmediate() < 4);

    synca::waitForAll();

    BOOST_TEST(counter.getImmediate() == 4);
}

template <template <typename...> typename T_co>
void testCoAsyncAwait()
{
    SyncaFixture::start("testCoAsyncAwait", [] {
        T_co<SlowCounter, subjector::CoAsyncAwait> counter;
        for (int i = 0; i < 100; ++i) {
            counter.async().inc();
        }

        counter.wait();

        BOOST_TEST(counter.get() == 100);
    });
}

template <template <typename...> typename T_co>
void testCoAsyncAwaitParallel()
{
    SyncaFixture::start("testCoAsyncAwaitParallel", [] {
        T_co<SlowCounterAtomic, subjector::CoAsyncAwait> counter;
        for (int i = 0; i < 100; ++i)
            counter.async().inc();

        BOOST_TEST(counter.getImmediate() < 100);

        counter.wait();

        BOOST_TEST(counter.getImmediate() == 100);

        for (int i = 0; i < 100; ++i)
            counter.async().inc();

        BOOST_TEST(counter.getImmediate() < 200);

        counter.wait();

        BOOST_TEST(counter.getImmediate() == 200);
    });
}

#define SUBJECTOR_TEST_CASE(D_subjector, D_case)                                         \
    BOOST_AUTO_TEST_CASE(D_subjector##D_case)                                            \
    {                                                                                    \
        testCo##D_case<subjector::D_subjector>();                                        \
    }

#define SUBJECTOR_NO_SCHEDULER_CASES(D_subjector)                                        \
    SUBJECTOR_TEST_CASE(D_subjector, )                                                   \
    SUBJECTOR_TEST_CASE(D_subjector, Async)                                              \
    SUBJECTOR_TEST_CASE(D_subjector, AsyncParallel)                                      \
    SUBJECTOR_TEST_CASE(D_subjector, AsyncAwait)                                         \
    SUBJECTOR_TEST_CASE(D_subjector, AsyncAwaitParallel)

#define SUBJECTOR_WITH_ASYNC_SCHEDULER_CASES(D_subjector)                                \
    SUBJECTOR_NO_SCHEDULER_CASES(D_subjector)                                            \
    SUBJECTOR_TEST_CASE(D_subjector, AsyncScheduler)

#define SUBJECTOR_ALL_CASES(D_subjector)                                                 \
    SUBJECTOR_WITH_ASYNC_SCHEDULER_CASES(D_subjector)                                    \
    SUBJECTOR_TEST_CASE(D_subjector, Scheduler)

#define SUBJECTOR_GO_CASES(D_subjector)                                                  \
    SUBJECTOR_TEST_CASE(D_subjector, )                                                   \
    SUBJECTOR_TEST_CASE(D_subjector, Scheduler)                                          \
    SUBJECTOR_TEST_CASE(D_subjector, AsyncAwait)                                         \
    SUBJECTOR_TEST_CASE(D_subjector, AsyncAwaitParallel)
