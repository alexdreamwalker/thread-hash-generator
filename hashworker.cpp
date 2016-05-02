#include "hashworker.h"
#ifdef DEBUG_MODE
#include <iostream>
#endif

HashWorker::HashWorker() : workersCount(0), hashQueue(), dataLeft(true)
{
    /* Connect signal of completion of thread with a function of decreasing active threads number */
    workerThreadReady.connect(boost::bind(&HashWorker::taskFinished, this));
}

/* Get CRC of string */
void HashWorker::getCRC32(std::shared_ptr<NumberedString> data) {
    boost::crc_32_type result;
    result.process_bytes(data->data.data(), data->data.length());
    data->data = std::to_string(result.checksum());
}

/* Make hash of each block (each in a separate thread) */
void HashWorker::getHash(std::shared_ptr<NumberedString> task)
{
#ifdef DEBUG_MODE
    std::cout << "Hashing #" << task->number << std::endl;
#endif
    getCRC32(task);
    /* Send signal that hashing is ready and push into priority queue to write */
    hashReady(task);
    /* Send signal that thread task is done */
    workerThreadReady();
    hashQueueMutex.lock();
    workersCountMutex.lock();
    /* If there are no other active hashing threads, hashing queue is empty and all data has beeb read */
    if(!dataLeft && hashQueue.empty() && workersCount == 0) {
#ifdef DEBUG_MODE
        std::cout << "HASH thread FINISHED" << std::endl;
#endif
        /* Send signal to main thread that hashing task was completed */
        allHashReady();
    }
    hashQueueMutex.unlock();
    workersCountMutex.unlock();
}

/* Decrease the variable of currently working threads */
void HashWorker::taskFinished()
{
    workersCountMutex.lock();
    workersCount--;
    workersCountMutex.unlock();
    checkQueue();
}

/* No more data will be pushed in hashing queue. Triggered by signal from reading thread */
void HashWorker::readFinished()
{
    dataLeft = false;
    /* If there are no other active hashing threads and hashing queue is empty, notify main thread that hashing job is completed */
    if(hashQueue.empty() && workersCount == 0)
    {
#ifdef DEBUG_MODE
        std::cout << "HASH thread FINISHED" << std::endl;
#endif
        allHashReady();
    }
}

/* Check hashing queue */
void HashWorker::checkQueue()
{
    hashQueueMutex.lock();
    if(!hashQueue.empty())
    {
        workersCountMutex.lock();
        if(workersCount <= MAX_HASH_WORKERS)
        {
            /* If current count of active threads is lower than max value, start new hashing thread */
            std::shared_ptr<NumberedString> ns = hashQueue.front();
            hashQueue.pop();
            workersCount++;
            std::thread thr(&HashWorker::getHash, this, ns);
            thr.detach();
        }
        else
        {
#ifdef DEBUG_MODE
            std::cout << "No more hash workers can be active" << std::endl;
#endif
        }
        workersCountMutex.unlock();
    }
    else
    {
#ifdef DEBUG_MODE
        std::cout << "Hash queue is empty" << std::endl;
#endif
    }
    hashQueueMutex.unlock();
}

/* Push new task into hashing queue. Triggered by signal from reading thread */
void HashWorker::getHashTask(std::shared_ptr<NumberedString> task)
{
    hashQueueMutex.lock();
    hashQueue.push(task);
    hashQueueMutex.unlock();
    checkQueue();
}


HashWorker::~HashWorker()
{

}
