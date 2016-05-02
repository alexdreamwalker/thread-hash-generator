#include <iostream>
#include <string>
#include <boost/program_options.hpp>
#include <boost/exception/diagnostic_information.hpp>

#include "signaturegenerator.h"

/* Default block size in Mbytes, used if there was no block size provided in command line or block size is incorrect */
#define DEFAULT_BLOCK_SIZE  1

namespace po = boost::program_options;

int main(int ac, char **av)
{
    /* Set and parse required params */
    po::options_description desc("Required parameters");
    desc.add_options()
            ("input-file", po::value<std::string>(), "file to read")
            ("output-file", po::value<std::string>(), "file to write")
            ("block-size", po::value<unsigned int>(), "block size (Mb)")
            ;
    po::variables_map vm;
    po::store(po::parse_command_line(ac, av, desc), vm);
    po::notify(vm);
    /* Set variables from command line */
    std::string inputFile;
    std::string outputFile;
    unsigned int blockSize;
    bool errorOccured = false;
    try
    {
        if(!vm.count("input-file"))
        {
            std::cout << "Error: no input file provided" << std::endl;
            errorOccured = true;
        }
        else
        {
            inputFile = vm["input-file"].as<std::string>();
        }

        if(!vm.count("output-file"))
        {
            std::cout << "Error: no output file provided" << std::endl;
            errorOccured = true;
        }
        else
        {
            outputFile = vm["output-file"].as<std::string>();
        }

        if(!vm.count("block-size"))
        {
            std::cout << "Warning: no block size provided. Default block size will be used" << std::endl;
            blockSize = DEFAULT_BLOCK_SIZE;
        }
        else
        {
            blockSize = vm["block-size"].as<unsigned int>();
            if(blockSize == 0)
            {
                std::cout << "Warning: Invalid block size provided. Default block size will be used" << std::endl;
                blockSize = DEFAULT_BLOCK_SIZE;
            }
        }
    }
    catch(boost::exception &error)
    {
        errorOccured = true;
        std::cout << "Error : " << boost::diagnostic_information(error) << std::endl;
    }
    if(errorOccured)
    {
        /* Show error info and exit */
        std::cout << desc << std::endl;
        exit(1);
    }

    std::cout << "Beginning with the help of " << std::thread::hardware_concurrency() << " threads..." << std::endl;
    /* Start main task */
    SignatureGenerator signatureGenerator(inputFile, outputFile, (blockSize * 1024 * 1024));
    signatureGenerator.start();
    /* Finish main task */
    std::cout << "Ready with the help of " << std::thread::hardware_concurrency() << " threads" << std::endl;
    return 0;
}
