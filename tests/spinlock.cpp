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

#include <synca/synca.h>

#include "counter.h"
#include "ut.h"

BOOST_FIXTURE_TEST_SUITE(Spinlock, SyncaFixture)

BOOST_AUTO_TEST_CASE(SpinlockLoop)
{
    synca::DefaultThreadPool tp{4, "SpinlockLoop"};
    SlowCounter c;
    BarrierCounter b;
    BarrierCounter bMain;
    synca::Mutex mut;
    for (int i = 0; i < 4; ++i)
        synca::go([&] {
            bMain.unblock();
            b.wait(0);
            for (int i = 0; i < 100; ++i) {
                synca::Lock _{mut};
                c.inc();
            }
        });
    bMain.wait(3);
    b.unblock();
    synca::waitForAll();
    BOOST_CHECK_EQUAL(c.get(), 100 * 4);
}

BOOST_AUTO_TEST_SUITE_END()
