#include "Arguments.h"

RemoteV8::Arguments::Arguments(int argc, char* argv[])
{
	// Program command line arguments:
	// 1. path to script
	// 2. path to csv data
	// 3. path to output file
	// 4. number of v8 threads

	if (argc >= 5)
	{
		this->scriptFilename = std::string(argv[1]);
		this->csvInputFilename = std::string(argv[2]);
		this->logFilename = std::string(argv[3]);

		int v8t = 0, qs = 0;
		sscanf_s(argv[4], "%d", &v8t);
		this->v8threads = v8t;

		this->data.open(this->csvInputFilename, std::ifstream::in);
		this->log.open(this->logFilename, std::ofstream::out);
		this->inputDataStreamEnded = false;
		
		std::ifstream scriptStream(this->scriptFilename, std::ifstream::in);
		if (scriptStream) 
		{
			scriptStream.seekg(0, scriptStream.end);
			auto length = scriptStream.tellg();
			scriptStream.seekg(0, scriptStream.beg);
			char *buffer = new char[length];
			for (int i = 0; i < length; i++) buffer[i] = 0;
			scriptStream.read(buffer, length);
			this->script = std::string(buffer);
			delete[] buffer;
			scriptStream.close();
		}
	}
	else
	{

	}
}