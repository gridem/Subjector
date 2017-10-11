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

#pragma once

#include "oers.h"
#include "schedule.h"

#include <limits>

#include <boost/circular_buffer.hpp>
#include <boost/intrusive/list.hpp>
#include <boost/intrusive/list_hook.hpp>
#include <boost/utility/in_place_factory.hpp>

namespace synca {

template <typename T>
struct Channel
{
    using Result = boost::optional<T>;
    using Results = std::vector<T>;

    struct Iterator
    {
        Iterator() = default;
        Iterator(Channel& c)
            : ch(&c)
        {
            ++*this;
        }

        T& operator*()
        {
            return *val;
        }

        Iterator& operator++()
        {
            val = ch->get();
            if (!val) {
                ch = nullptr;
            }
            return *this;
        }

        bool operator!=(const Iterator& i) const
        {
            return ch != i.ch;
        }

    private:
        Result val;
        Channel* ch = nullptr;
    };

    Iterator begin()
    {
        return {*this};
    }
    static Iterator end()
    {
        return {};
    }

    // by default the channel is unlimited
    Channel()
        : buffer_{std::numeric_limits<size_t>::max()}
    {
    }

    explicit Channel(size_t lim)
        : buffer_{lim}
    {
    }

    ~Channel()
    {
        close();
    }

    Result get()
    {
        auto r = get(1);
        if (r.empty())
            return {};
        return r[0];
    }

    Results get(size_t num)
    {
        Results rs;
        rs.reserve(num);
        get(std::back_inserter(rs), num);
        return rs;
    }

    bool put(const T& t)
    {
        return put(&t, 1);
    }

    bool put(T&& t)
    {
        return put(std::make_move_iterator(&t), 1);
    }

    template <typename T_iterator>
    bool put(T_iterator begin, size_t num)
    {
        std::unique_lock<std::mutex> _{mutex_};
        if (closed_)
            return false;
        boost::optional<Waiter> w;
        while (num != 0) {
            while (buffer_.full()) {
                if (!w)
                    w = boost::in_place(&putWaiters_);
                _.unlock();
                w->wait();
                _.lock();
                if (closed_)
                    return false;
            }
            size_t available = buffer_.capacity() - buffer_.size();
            size_t operational = std::min(num, available);
            pushFrom(begin, operational);
            begin += operational;
            num -= operational;
            getWaiters_.wakeup();
        }
        return true;
    }

    template <typename T_iterator>
    bool get(T_iterator begin, size_t num)
    {
        std::unique_lock<std::mutex> _{mutex_};
        if (closed_)
            return false;
        boost::optional<Waiter> w;
        while (num != 0) {
            while (buffer_.empty()) {
                if (!w)
                    w = boost::in_place(&getWaiters_);
                _.unlock();
                w->wait();
                _.lock();
                if (closed_)
                    return false;
            }
            size_t available = buffer_.size();
            size_t operational = std::min(num, available);
            popTo(begin, operational);
            // the common way is to use back_inserter
            // thus we don't need to update the begin iterator: begin += operational;
            num -= operational;
            putWaiters_.wakeup();
        }
        return true;
    }

    template <typename T_iterator>
    size_t getNonblocking(T_iterator begin, size_t num)
    {
        std::unique_lock<std::mutex> _{mutex_};
        if (closed_)
            return 0;
        size_t available = buffer_.size();
        size_t operational = std::min(num, available);
        popTo(begin, operational);
        putWaiters_.wakeup();
        return operational;
    }

    bool empty() const
    {
        std::unique_lock<std::mutex> _{mutex_};
        return buffer_.empty();
    }

    void close()
    {
        std::unique_lock<std::mutex> _{mutex_};
        if (closed_)
            return;
        closed_ = true;
        for (auto&& w : getWaiters_)
            w.done();
    }

    // Note: it's dangerous to change the limit on-the-fly
    // because some data may be dropped on size decreasing
    // if the container has some data
    void limit(size_t lim)
    {
        std::unique_lock<std::mutex> _{mutex_};
        buffer_.set_capacity(lim);
    }

    void unlimit()
    {
        std::unique_lock<std::mutex> _{mutex_};
        buffer_.set_capacity(std::numeric_limits<size_t>::max());
    }

private:
    struct Waiter : Doer,
                    boost::intrusive::list_base_hook<
                        boost::intrusive::link_mode<boost::intrusive::auto_unlink>>
    {
        template <typename T_container>
        Waiter(T_container* c)
        {
            c->push_back(*this);
        }
    };

    struct Waiters
        : boost::intrusive::list<Waiter, boost::intrusive::constant_time_size<false>>
    {
        void wakeup()
        {
            if (!this->empty())
                this->front().done();
        }
    };

    template <typename T_iterator>
    void pushFrom(T_iterator t, size_t n)
    {
        buffer_.insert(buffer_.end(), t, t + n);
    }

    template <typename T_outputIterator>
    void popTo(T_outputIterator t, size_t n)
    {
        std::move(buffer_.begin(), buffer_.begin() + n, t);
        buffer_.rresize(buffer_.size() - n);
    }

    mutable std::mutex mutex_;
    bool closed_ = false;
    boost::circular_buffer_space_optimized<T> buffer_;
    Waiters getWaiters_;
    Waiters putWaiters_;
};

} // namespace synca
