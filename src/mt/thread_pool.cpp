/*
 * Copyright 2014-2017 Grigory Demchenko (aka gridem)
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

#include "mt_impl.h"

#define LOG_COMPONENT_TP LOG_COMPONENT_MT << "@" << this->name() << ": "

namespace mt {
namespace detail {

struct Service : boost::asio::io_service
{
};

} // namespace detail

struct ThreadPool::Impl
{
    using Work = detail::Service::work;

    Impl(size_t threadCount, const char* name)
        : tpName_{name}
    {
        createWork();
        threads_.reserve(threadCount);
        for (size_t i = 0; i < threadCount; ++i)
            threads_.emplace_back(detail::createThread([this] { loop(); }, i, name));
        LOG(DEBUG, TP) << "thread pool created with threads: " << threadCount;
    }

    ~Impl()
    {
        Lock lock{mutex_};
        toStop_ = true;
        work_ = boost::none;
        lock.unlock();
        LOG(DEBUG, TP) << "stopping thread pool";
        for (size_t i = 0; i < threads_.size(); ++i)
            threads_[i].join();
        LOG(DEBUG, TP) << "thread pool stopped";
    }

    void wait()
    {
        Lock lock{mutex_};
        work_ = boost::none;
        while (true) {
            cond_.wait(lock);
            LOG(DEBUG, TP) << "WAIT: waitCompleted: " << !!work_;
            if (work_)
                break;
        }
    }

    template <typename F>
    void post(F&& f)
    {
        service_.post(std::forward<F>(f));
    }

    const char* name() const
    {
        return tpName_;
    }

    detail::Service& service()
    {
        return service_;
    }

private:
    void createWork()
    {
        work_.emplace(service_);
    }

    void loop()
    {
        while (true) {
            service_.run();
            Lock lock{mutex_};
            if (toStop_)
                break;
            if (!work_) {
                createWork();
                service_.reset();
                lock.unlock();
                cond_.notify_all();
            }
        }
    }

    const char* tpName_;
    detail::Service service_;
    boost::optional<Work> work_;
    std::vector<std::thread> threads_;
    std::mutex mutex_;
    std::condition_variable cond_;
    bool toStop_ = false;
};

ThreadPool::ThreadPool(size_t threadCount, const char* name)
    : impl{std::make_unique<Impl>(threadCount, name)}
{
}

ThreadPool::~ThreadPool()
{
}

void ThreadPool::schedule(Handler handler)
{
    impl->post(std::move(handler));
}

void ThreadPool::wait()
{
    impl->wait();
}

const char* ThreadPool::name() const
{
    return impl->name();
}

detail::Service& ThreadPool::service()
{
    return impl->service();
}

} // namespace mt
