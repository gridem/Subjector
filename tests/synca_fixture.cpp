#include <synca/synca.h>

#include "synca_fixture.h"

void SyncaFixture::start(const char* name, Handler h, size_t threads)
{
    synca::DefaultThreadPool tp{threads, name};
    synca::go(std::move(h));
    synca::waitForAll();
}

void SyncaFixture::sleep()
{
    mt::sleepFor(5ms);
}

void SyncaFixture::sleepFast()
{
    mt::sleepFor(200us);
}
