#include "outputworker.h"
#include <iostream>

#define DEFAULT_FILENAME    "OUTPUT.DAT"
#define DEFAULT_BLOCKSIZE   1


OutputWorker::OutputWorker() : path(DEFAULT_FILENAME), outFile(path.string().c_str()), currentIndex(0), writeQueue(), dataLeft(true), writingLeft(true)
{
    /* Send signal to set flag that all writing tasks were completed */
    allWriteDone.connect(boost::bind(&OutputWorker::writingFinished, this));
}

OutputWorker::OutputWorker(std::string fileName) : path(fileName), outFile(path.string().c_str()), currentIndex(0), writeQueue(), dataLeft(true), writingLeft(true)
{
    /* Send signal to set flag that all writing tasks were completed */
    allWriteDone.connect(boost::bind(&OutputWorker::writingFinished, this));
}

void OutputWorker::startWrite()
{
    try
    {
        if(!outFile.is_open())
        {
            boost::throw_exception(boost::enable_error_info(std::runtime_error("CannotWriteFileError")) << tagErrorInfo("Output file cannot be written"));
        }
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        if(std::string const * extra  = boost::get_error_info<tagErrorInfo>(e))
        {
            std::cout << *extra << std::endl;
        }
        exit(1);
    }

    while(writingLeft)
    {
        /* Block current writing thred while there are no block with appropriate index */
        if(dataLeft)
        {
            std::unique_lock<std::mutex> lk(waitMutex);
            waitVariable.wait(lk);
        }
        checkQueue();
    }
}

/* No more data will be placed in writing priority queue. Triggered by signal from hashing threads */
void OutputWorker::hashingFinished()
{
    dataLeft = false;
    /* Unlock writing thread - nothing to wait */
    std::unique_lock<std::mutex> lk(waitMutex);
    lk.unlock();
    waitVariable.notify_one();
}

/* All blocks were written */
void OutputWorker::writingFinished()
{
    writingLeft = false;
}

/* Check priority queue */
void OutputWorker::checkQueueCondition()
{
    writeQueueMutex.lock();
#ifdef DEBUG_MODE
    std::cout << "Checking queue, size: " << writeQueue.size() << std::endl;
#endif
    if(writeQueue.empty())
    {
        writeQueueMutex.unlock();

        return;
    }
    std::shared_ptr<NumberedString> ns = writeQueue.top();
    writeQueueMutex.unlock();
    /* If there is data with appropriate index, unblock writing thread */
    if(ns->number == currentIndex)
    {
        std::unique_lock<std::mutex> lk(waitMutex);
        lk.unlock();
        waitVariable.notify_one();
    }
    else
    {
#ifdef DEBUG_MODE
        std::cout << "Cannot write #" << ns->number << " cause need #" << currentIndex << std::endl;
#endif
    }
}

/* Get data from writing priority queue */
void OutputWorker::checkQueue()
{
    writeQueueMutex.lock();
    if(writeQueue.empty())
    {
        writeQueueMutex.unlock();
        if(!dataLeft)
        {
        #ifdef DEBUG_MODE
            std::cout << "WRITE thread FINISHED" << std::endl;
        #endif
            /* If there is no data, send signal that all writing was finished */
            allWriteDone();
        }
        return;
    }
    /* Get one element from the top of writing queue - the condition of correct index was already checked */
    std::shared_ptr<NumberedString> ns = writeQueue.top();
    writeQueue.pop();
    writeQueueMutex.unlock();
#ifdef DEBUG_MODE
    std::cout << "Writing #" << ns->number << std::endl;
#endif
    /* And write it to output file */
    writeBlock(ns);
}

/* Push data in priority queue for writing. Triggered by signal from hashing threads */
void OutputWorker::queueBlock(std::shared_ptr<NumberedString> block)
{
    writeQueueMutex.lock();
#ifdef DEBUG_MODE
    std::cout << "Pushing #" << block->number << " queue size is " << writeQueue.size() << std::endl;
#endif
    writeQueue.push(block);
    writeQueueMutex.unlock();
    checkQueueCondition();
}

/* Write data to output file */
void OutputWorker::writeBlock(std::shared_ptr<NumberedString> block)
{
    try
    {
        outFile.write(block->data.c_str(), block->data.length());
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        exit(1);
    }
    currentIndexMutex.lock();
    currentIndex++;
    currentIndexMutex.unlock();
}

OutputWorker::~OutputWorker()
{
    outFile.close();
}
