#ifndef REM_V8_ARGS
#define REM_V8_ARGS

#include "stdafx.h"

namespace RemoteV8 
{
	// Program command line arguments:
	// 1. path to script
	// 2. path to csv data
	// 3. path to output file
	// 4. number of v8 threads
	// 5. size of input and output queues.
	class Arguments
	{
		std::string scriptFilename;
		std::string csvInputFilename;
		std::string logFilename;

		std::string script;
		std::ofstream log;
		std::ifstream data;

		boost::mutex logMutex;
	public:
		Arguments(int argc, char* argv[]);
		std::string getScript() { return this->script; }
		std::ifstream& getCsvDataStream() { return this->data; }
		std::ofstream& getLogStream() { return this->log; }
		void v8threadsDecrease() { 
			logMutex.lock();
			this->v8threads--; 
			logMutex.unlock();
		}

	public:	
		bool inputDataStreamEnded;
		int v8threads;
	};
}


#endif // !REM_V8_ARGS