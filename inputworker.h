#ifndef __INPUT_WORKER_H__
#define __INPUT_WORKER_H__

#include <boost/filesystem.hpp>
#include <boost/signals2.hpp>
#include <memory>
#include <string>
#include <fstream>
#include <boost/throw_exception.hpp>
#include <boost/exception/all.hpp>

#include "numbereddata.hpp"

typedef boost::error_info<struct my_tag,std::string> tagErrorInfo;

class InputWorker
{
private:
    /* Path to input file */
    boost::filesystem::path path;
    /* Required size of block */
    unsigned int blockSize;
    /* Index of current block */
    unsigned int currentIndex;
public:
    InputWorker();
    InputWorker(std::string fileName, unsigned int bs);
    /* New block was read from file */
    boost::signals2::signal<void(std::shared_ptr<NumberedString>)> blockReadDone;
    /* All blocks were read from file */
    boost::signals2::signal<void()> allReadDone;
    void startRead();
    ~InputWorker();
};

#endif
