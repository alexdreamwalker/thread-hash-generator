#include "inputworker.h"
#include <iostream>

#define DEFAULT_FILENAME    "INPUT.DAT"
#define DEFAULT_BLOCKSIZE   1024

InputWorker::InputWorker() : path(DEFAULT_FILENAME), blockSize(DEFAULT_BLOCKSIZE), currentIndex(0)
{

}

InputWorker::InputWorker(std::string fileName, unsigned int bs) : path(fileName), blockSize(bs), currentIndex(0)
{

}

/* Start reading blocks task */
void InputWorker::startRead()
{
    std::ifstream inFile(path.string().c_str(), std::ios::in | std::ios::binary);
    try
    {
        if(inFile.is_open())
        {
            currentIndex = 0;
            std::string buffer(blockSize, 0);
            while(true)
            {
                /* Fill the buffer with zeros in order to handle blocks which size is lower than block size */
                std::fill(buffer.begin(), buffer.end(), 0);
                /* Read a block of data from file */
                if(inFile.read(&buffer[0], blockSize))
                {
#ifdef DEBUG_MODE
                    std::cout << "Reading #" << currentIndex << std::endl;
#endif
                    /* Make a struct with data and corresponding index and take smart pointer */
                    std::shared_ptr<NumberedString> block(new NumberedString(currentIndex, buffer));
                    /* Send a signal of new hashing task with smart pointer */
                    blockReadDone(block);
                    currentIndex++;
                }
                else
                {
                    break;
                }
            }
            inFile.close();
#ifdef DEBUG_MODE
            std::cout << "READ thread FINISHED" << std::endl;
#endif
            /* Send a signal to main thread that all blocks were read */
            allReadDone();
        }
        else
        {
            /* Create an exception if input file cannot be read */
            boost::throw_exception(boost::enable_error_info(std::runtime_error("CannotReadFileError")) << tagErrorInfo("Input file can not be read"));
        }
    }
    catch(const std::exception& e)
    {
        /* Show error info and exit */
        std::cerr << e.what() << std::endl;
        if(std::string const * extra  = boost::get_error_info<tagErrorInfo>(e))
        {
            std::cout << *extra << std::endl;
        }
        exit(1);
    }
}

InputWorker::~InputWorker()
{

}
