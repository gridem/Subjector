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

struct User
{
    using Id = int;

    template <typename T>
    void addFriend(T& myFriend)
    {
        SyncaFixture::sleep();
        auto friendId = myFriend.getId();
        if (hasFriend(friendId)) {
            return;
        }
        addFriendId(friendId);
        auto myId = getId();
        myFriend.addFriendId(myId);
    }

    Id getId() const
    {
        return 0;
    }

    void addFriendId(Id)
    {
    }

private:
    bool hasFriend(Id)
    {
        return false;
    }
};

struct UserAlone : User
{
};

DECL_ADAPTER(User, addFriend, getId, addFriendId)

DECL_ADAPTER(UserAlone, addFriend, getId, addFriendId)
BIND_SUBJECTOR(UserAlone, CoAlone)

template <typename T>
void makeFriends(T& u1, T& u2)
{
    u1.addFriend(u2);
}

BOOST_FIXTURE_TEST_SUITE(CoAlone, SyncaFixture)

SUBJECTOR_ALL_CASES(CoAlone)

BOOST_AUTO_TEST_CASE(CoMutexDeadlock)
{
    start("CoMutexDeadlock", [&] {
        subjector::Subjector<User> u1, u2;
        Atomic<int> done;
        synca::Awaiter a;
        a.go([&] {
            makeFriends(u1, u2);
            ++done;
        });
        a.go([&] {
            makeFriends(u2, u1);
            ++done;
        });
        mt::sleepFor(100ms);
        BOOST_TEST(done == 0);
    });
}

BOOST_AUTO_TEST_CASE(CoAloneDeadlockFree)
{
    start("CoAloneDeadlockFree", [&] {
        subjector::Subjector<UserAlone> u1, u2;
        Atomic<int> done;
        synca::Awaiter a;
        a.go([&] {
            makeFriends(u1, u2);
            ++done;
        });
        a.go([&] {
            makeFriends(u2, u1);
            ++done;
        });
        mt::sleepFor(100ms);
        BOOST_TEST(done == 2);
    });
}

BOOST_AUTO_TEST_SUITE_END()
