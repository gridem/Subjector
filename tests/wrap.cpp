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

#include "ut.h"

#include <common.h>

BOOST_AUTO_TEST_CASE(WrapVoid)
{
    auto result = wrap([] {});
    result.unwrap();
    static_assert(std::is_same_v<decltype(result.unwrap()), void>);
}

BOOST_AUTO_TEST_CASE(WrapInt)
{
    auto result = wrap([] { return 2; });
    auto value = result.unwrap();
    BOOST_TEST(value == 2);
}
