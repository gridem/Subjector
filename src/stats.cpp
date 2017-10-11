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

#include <mutex>
#include <unordered_map>

#include <log.h>
#include <mt/mt.h>
#include <stats.h>

#define LOG_COMPONENT_STATS LOG_COMPONENT_MT

#ifdef flagGCC_LIKE
#include <cxxabi.h>

struct Free
{
    void operator()(void* v)
    {
        free(v);
    }
};

using CharPtr = std::unique_ptr<char, Free>;

std::string demangle(const char* name)
{
    int status;
    CharPtr demangled{abi::__cxa_demangle(name, 0, 0, &status)};
    VERIFY(status == 0, "Invalid demangle status");
    return demangled.get();
}

#else

std::string demangle(const char* name)
{
    return name;
}

#endif

namespace synca {

struct StatRegistrar
{
    void registerValue(const char* name, StatCounter* v);
    void dump();

private:
    using Lock = std::unique_lock<std::mutex>;
    mutable std::mutex mutex_;
    std::unordered_map<std::string, StatCounter*> infos_;
};

void post(Handler h)
{
    h();
}

void detail::registerStat(const char* name, StatCounter* v)
{
    post([name, v] {
        try {
            single<StatRegistrar>().registerValue(name, v);
        } catch (std::exception& e) {
            LOG(ERROR, STATS) << "Exception on stat registering: " << e.what();
        }
    });
}

void replaceAll(std::string& s, const std::string& rm, const std::string& ad = "")
{
    while (true) {
        size_t p = s.find(rm);
        if (p == std::string::npos)
            return;
        s.replace(p, rm.size(), ad);
    }
}

std::string remove(const std::string& s, std::initializer_list<const char*> lst)
{
    std::string r = s;
    for (auto&& l : lst)
        replaceAll(r, l);
    return r;
}

void StatRegistrar::registerValue(const char* name, synca::StatCounter* v)
{
    std::string reducedName =
        remove(demangle(name), {"struct ", "synca::", "InstanceStat", "Stat", "<", ">"});
    Lock _{mutex_};
    VERIFY(infos_.find(reducedName) == infos_.end(), "Statistics name must be unique");
    infos_[reducedName] = v;
}

void StatRegistrar::dump()
{
    Lock _{mutex_};
    LOG(INFO, STATS) << "dumping statistics";
    for (auto&& v : infos_) {
        LOG(INFO, STATS) << "--- stat: " << v.first << ": "
                         << v.second->load(std::memory_order_relaxed);
    }
}

void dumpStats()
{
    single<StatRegistrar>().dump();
}

} // namespace synca
