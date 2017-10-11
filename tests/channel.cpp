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

BOOST_FIXTURE_TEST_SUITE(Channel, SyncaFixture)

BOOST_AUTO_TEST_CASE(ChannelCreate)
{
    synca::Channel<int> c;
}

BOOST_AUTO_TEST_CASE(ChannelSync)
{
    start("ChannelSync", [] {
        synca::Channel<int> c;
        c.put(1);
        BOOST_CHECK_EQUAL(*c.get(), 1);
    });
}

BOOST_AUTO_TEST_CASE(ChannelSyncLoop)
{
    start("ChannelSyncLoop", [] {
        synca::Channel<int> c;
        for (int i = 0; i < 100; ++i)
            c.put(i);
        for (int i = 0; i < 100; ++i)
            BOOST_CHECK_EQUAL(*c.get(), i);
    });
}

BOOST_AUTO_TEST_CASE(ChannelAsync)
{
    start("ChannelAsync", [] {
        BarrierCounter b;
        synca::Channel<int> c;
        synca::Awaiter a;
        a.go([&] {
            for (int i = 0; i < 100; ++i) {
                b.wait(0);
                c.put(i);
            }
        });
        a.go([&] {
            for (int i = 0; i < 100; ++i) {
                BOOST_CHECK_EQUAL(*c.get(), i);
            }
        });
        sleep();
        b.unblock();
        a.wait();
    });
}

BOOST_AUTO_TEST_CASE(ChannelAsyncAll)
{
    constexpr int count = 10;
    start("ChannelAsyncAll", [] {
        BarrierCounter b;
        synca::Channel<int> c;
        synca::Awaiter a;
        a.go([&] {
            for (int i = 0; i < count; ++i) {
                b.wait(i);
                c.put(i);
            }
        });
        a.go([&] {
            for (int i = 0; i < count; ++i) {
                BOOST_CHECK_EQUAL(*c.get(), i);
            }
        });
        for (int i = 0; i < count; ++i) {
            sleep();
            b.unblock();
        }
        a.wait();
    });
}

BOOST_AUTO_TEST_CASE(ChannelMultiProducers)
{
    constexpr int count = 30;
    start("ChannelMultiProducers", [] {
        synca::Channel<int> c;
        synca::Awaiter a;
        for (int i = 0; i < count; ++i) {
            a.go([&] {
                for (int i = 0; i < count; ++i) {
                    c.put(i);
                }
            });
        }
        a.wait();
        int sum = 0;
        for (int i = 0; i < count * count; ++i) {
            sum += *c.get();
        }
        BOOST_CHECK_EQUAL(sum, count * (count - 1) * count / 2);
    });
}

BOOST_AUTO_TEST_CASE(ChannelMultiConsumers)
{
    constexpr int count = 30;
    start("ChannelMultiConsumers", [] {
        synca::Channel<int> c;
        synca::Awaiter a;
        for (int i = 0; i < count * count; ++i) {
            c.put(i % count);
        }
        Atomic<int> total;
        for (int i = 0; i < count; ++i) {
            a.go([&] {
                int sum = 0;
                for (int i = 0; i < count; ++i) {
                    sum += *c.get();
                }
                total += sum;
            });
        }
        a.wait();
        BOOST_CHECK_EQUAL(total.load(), count * (count - 1) * count / 2);
    });
}

BOOST_AUTO_TEST_CASE(ChannelMultiProducersMultiConsumers)
{
    constexpr int count = 100;
    start("ChannelMultiProducersMultiConsumers", [] {
        synca::Channel<int> c;
        synca::Awaiter a;
        Atomic<int> total;
        for (int i = 0; i < count; ++i) {
            a.go([&] {
                int sum = 0;
                for (int i = 0; i < count; ++i) {
                    sum += *c.get();
                }
                total += sum;
            });
            a.go([&] {
                for (int i = 0; i < count; ++i) {
                    c.put(i);
                }
            });
        }
        a.wait();
        BOOST_CHECK_EQUAL(total.load(), count * (count - 1) * count / 2);
    });
}

BOOST_AUTO_TEST_CASE(ChannelForLoop)
{
    static constexpr int count = 100;
    start("ChannelForLoop", [] {
        synca::Channel<int> c;
        synca::go([&] {
            for (int i = 0; i < count; ++i) {
                c.put(i);
            }
            mt::sleepFor(10ms);
            c.close();
        });
        int j = 0;
        for (auto&& v : c) {
            BOOST_TEST(v == j++);
        }
        BOOST_TEST(j == count);
    });
}

BOOST_AUTO_TEST_SUITE_END()
