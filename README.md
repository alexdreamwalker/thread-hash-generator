# thread-hash-generator
Multi-threading app to read a file and make a hash signature in another file

Build system: CMake
Libraries used: boost(system thread exception filesystem program_options signals2 crc priority_queue)
C++11 features are used for smart pointers, threads, locks etc..

The application checks for all required params at start time, after that creates an instance of SignatureGenerator class for control and connection of other threads.
InputWorker thread opens the input file and starts to read blocks of data. Smart pointers to structure containing corresponding data and order number are made of readen blocks of data. The order number is used later for combining the output structure in the correct order. After that the smart pointer is placed in a queue for hashing. After the finish of reading of all the blocks the thread sends the signal to the SignatureGenerator control object. 
HashWorker thread is created for each object in hashing queue, which hashes all object data and places it together with order number into priority queue for writing (priority queue for writing a signature  to the output file in a correct order). Maximum number of threads is defined by a constant, which has a default value of number of logical processors in target system  std::thread::hardware_concurrency(). If the number of threads is bigger of that value, a new hash thread will not be created until one of the existing threads will release. If all the tasks for hashing are done and the signal of the finish of the reading thread is gained by the control object (new tasks will not come), the signal is sent to SignatureGenerator control object.
The OutputWorker thread controls the output of the signature to file. It saves the order number of the last written data and blocks until there is an element with next order number in the priority queue. If all tasks for output are completed and the signal of hashing finished is gained by the control thread (there will be no other output tasks), a signal about finish of output is sent to the SignatureGenerator control thread.
The main control object SignatureGenerator starts other thread and blocks until signals about finish of reading, hashing and writing are gained. 
