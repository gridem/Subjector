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

BOOST_FIXTURE_TEST_SUITE(CoPortal, SyncaFixture)

BOOST_AUTO_TEST_CASE(CoSingleThreadPortal)
{
    synca::DefaultThreadPool tp{4, "Common"};
    mt::ThreadPool tp2{1, "Serialized"};
    BarrierCounter b;
    BarrierCounter bMain;
    subjector::CoPortal<SlowCounter> counter(tp2);
    for (int i = 0; i < 4; ++i)
        synca::go([&] {
            bMain.unblock();
            b.wait(0);
            for (int i = 0; i < 100; ++i) {
                counter.inc();
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

SUBJECTOR_NO_SCHEDULER_CASES(CoSerializedPortal)

BOOST_AUTO_TEST_SUITE_END()
