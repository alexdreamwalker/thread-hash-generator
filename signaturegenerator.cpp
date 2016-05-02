#include "signaturegenerator.h"
#ifdef DEBUG_MODE
#include <iostream>
#endif

SignatureGenerator::SignatureGenerator() : inputWorker(), outputWorker(), hashWorker(), isReadingFinished(false), isHashingFinished(false), isWritingFinished(false)
{

}

SignatureGenerator::SignatureGenerator(std::string inputFile, std::string outputFile, unsigned int blockSize) : inputWorker(inputFile, blockSize), outputWorker(outputFile), hashWorker(), isReadingFinished(false), isHashingFinished(false), isWritingFinished(false)
{

}

void SignatureGenerator::readingFinished()
{
    isReadingFinished = true;
    checkExitState();
}

void SignatureGenerator::hashingFinished()
{
    isHashingFinished = true;
    checkExitState();
}

void SignatureGenerator::writingFinished()
{
    isWritingFinished = true;
    checkExitState();
}

void SignatureGenerator::checkExitState()
{
    /* If all tasks were completed, unblock the main thread  */
    if(isReadingFinished && isHashingFinished && isWritingFinished)
    {
        std::unique_lock<std::mutex> lk(waitMutex);
        lk.unlock();
        waitVariable.notify_one();
    }
}

void SignatureGenerator::start()
{
    /* Connect signals and slots of different workers */
    inputWorker.blockReadDone.connect(boost::bind(&HashWorker::getHashTask, &hashWorker, _1));
    inputWorker.allReadDone.connect(boost::bind(&HashWorker::readFinished, &hashWorker));
    inputWorker.allReadDone.connect(boost::bind(&SignatureGenerator::readingFinished, this));

    hashWorker.hashReady.connect(boost::bind(&OutputWorker::queueBlock, &outputWorker, _1));
    hashWorker.allHashReady.connect(boost::bind(&OutputWorker::hashingFinished, &outputWorker));
    hashWorker.allHashReady.connect(boost::bind(&SignatureGenerator::hashingFinished, this));

    outputWorker.allWriteDone.connect(boost::bind(&SignatureGenerator::writingFinished, this));

    /* Create reading and writing threads (hashing threads are created dynamically) */
    std::thread outputThread(&OutputWorker::startWrite, &outputWorker);
    std::thread inputThread(&InputWorker::startRead, &inputWorker);
    std::unique_lock<std::mutex> lk(waitMutex);
    outputThread.detach();
    inputThread.detach();
    /* Block the main thread */
    waitVariable.wait(lk);
#ifdef DEBUG_MODE
    std::cout << "SG thread FINISHED" << std::endl;
#endif
}

