#ifndef __HASH_WORKER_H__
#define __HASH_WORKER_H__

#include <memory>
#include <string>
#include <queue>
#include <thread>
#include <mutex>
#include <boost/signals2.hpp>
#include <boost/crc.hpp>

#include "numbereddata.hpp"

/* Set max number of threads as a number of logical processors in system */
#define MAX_HASH_WORKERS    std::thread::hardware_concurrency()

class HashWorker
{
private:
    void getHash(std::shared_ptr<NumberedString> task);
    /* Mutex for accessing hashing queue */
    std::mutex hashQueueMutex;
    /* Hashing queue to store data that needs to be hashed */
    std::queue<std::shared_ptr<NumberedString>> hashQueue;
    /* Mutex for a number of active hashing threads */
    std::mutex workersCountMutex;
    /* Number of active hashing threads */
    unsigned int workersCount;
    /* Flag that there will be data from reading thread to hash */
    bool dataLeft;
    void checkQueue();
    void getCRC32(std::shared_ptr<NumberedString> data);
public:
    HashWorker();
    void getHashTask(std::shared_ptr<NumberedString> task);
    void taskFinished();
    void readFinished();
    /* Signal that one block of data was hashed */
    boost::signals2::signal<void(std::shared_ptr<NumberedString>)> hashReady;
    /* Signal that one hash worker thread completed its task and exited */
    boost::signals2::signal<void()> workerThreadReady;
    /* Signal that all blocks were hashed */
    boost::signals2::signal<void()> allHashReady;
    ~HashWorker();
};

#endif
