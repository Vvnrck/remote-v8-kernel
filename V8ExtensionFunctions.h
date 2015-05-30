//#ifndef REM_V8_EXTENSIONS
//#define REM_V8_EXTENSIONS

#include "stdafx.h"
#include "include/v8.h"
#include "Arguments.h"
#include "LockableQueue.h"
#include <fstream>
#include <iostream>
#include <boost/lockfree/queue.hpp>

namespace v8Extensions 
{

	static std::string logFilename;
	static void LogCallback(const v8::FunctionCallbackInfo<v8::Value>& args) 
	{
		if (args.Length() < 1) return;
		v8::HandleScope scope(args.GetIsolate());
		v8::Handle<v8::Value> arg = args[0];
		v8::String::Utf8Value value(arg);

		std::ofstream output(logFilename, std::ios_base::ate);
		output << *value << std::endl;
		output.close();
	}

	static boost::lockfree::queue<std::string*, boost::lockfree::fixed_sized<false>> outcomingData(100);
	static void toOutputCallback(const v8::FunctionCallbackInfo<v8::Value>& args)
	{
		if (args.Length() < 1) return;
		v8::HandleScope scope(args.GetIsolate());
		v8::Handle<v8::Value> arg = args[0];
		v8::String::Utf8Value value(arg);
		std::string* data = new std::string(*value);
		outcomingData.push(data);
	}
}




//#endif  // !REM_V8_EXTENSIONS