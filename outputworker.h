#ifndef __OUTPUT_WORKER_H__
#define __OUTPUT_WORKER_H__

#include <boost/filesystem.hpp>
#include <boost/signals2.hpp>
#include <boost/heap/priority_queue.hpp>
#include <string>
#include <fstream>
#include <mutex>
#include <memory>
#include <condition_variable>
#include <boost/throw_exception.hpp>
#include <boost/exception/all.hpp>


#include "numbereddata.hpp"

typedef boost::error_info<struct my_tag,std::string> tagErrorInfo;

class OutputWorker
{
private:
    /* Path to output file */
    boost::filesystem::path path;
    /* Output stream */
    std::ofstream outFile;
    /* Mutex for accessing current block index */
    std::recursive_mutex currentIndexMutex;
    /* Current index of block that should be written */
    unsigned int currentIndex;
    /* Flag that there will be more data from hashing queue */
    bool dataLeft;
    /* Flag that there are still entries to write */
    bool writingLeft;
    /* Mutex for accessing priority queue of write tasks */
    std::recursive_mutex writeQueueMutex;
    /* Priority queue for writing */
    boost::heap::priority_queue<std::shared_ptr<NumberedString>, boost::heap::compare<NumberedStringCompare>> writeQueue;
    void writeBlock(std::shared_ptr<NumberedString> block);
    /* Mutex and condition variable to allow blocking thread while there is no appropriate data in queue */
    std::mutex waitMutex;
    std::condition_variable waitVariable;
    void checkQueue();
    void checkQueueCondition();
public:
    OutputWorker();
    OutputWorker(std::string fileName);
    /* Signal to main thread that all writing was done */
    boost::signals2::signal<void()> allWriteDone;
    void startWrite();
    void hashingFinished();
    void writingFinished();
    void queueBlock(std::shared_ptr<NumberedString> block);
    ~OutputWorker();
};

#endif
