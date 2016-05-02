#ifndef __SIGNATURE_GENERATOR_H__
#define __SIGNATURE_GENERATOR_H__

#include <string>
#include <thread>
#include <boost/signals2.hpp>
#include <condition_variable>

#include "inputworker.h"
#include "outputworker.h"
#include "hashworker.h"


class SignatureGenerator
{
private:
    /* Thread to read input file */
    InputWorker inputWorker;
    /* Thread to write output file */
    OutputWorker outputWorker;
    /* Threads for hashing blocks */
    HashWorker hashWorker;
    /* Flag that reading was finished */
    bool isReadingFinished;
    /* Flag that hashing was finished */
    bool isHashingFinished;
    /* Flag that writing was finished */
    bool isWritingFinished;
    /* Mutex and condition to allow to block this thread until other tasks will be completed */
    std::mutex waitMutex;
    std::condition_variable waitVariable;
public:
    SignatureGenerator();
    SignatureGenerator(std::string inputFile, std::string outputFile, unsigned int blockSize);
    void start();
    void readingFinished();
    void writingFinished();
    void hashingFinished();
    void checkExitState();
};

#endif
