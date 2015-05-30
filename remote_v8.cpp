// remote_v8.cpp : Defines the entry point for the console application.
//

#define forever while(true)

#include "stdafx.h"
#include "V8ExtensionFunctions.h"
#include "Arguments.h"
#include "LockableQueue.h"
#include <boost/algorithm/string.hpp>
#include <boost/lockfree/queue.hpp>

using namespace v8;

void v8ExampleCompute();
void readerThread(RemoteV8::Arguments &args,
				  boost::lockfree::queue<std::string*, boost::lockfree::fixed_sized<false>> &incomingData);
void workerThread(RemoteV8::Arguments &args,
				  boost::lockfree::queue<std::string*, boost::lockfree::fixed_sized<false>> &incomingData);
void writerThread(RemoteV8::Arguments &args);


int main(int argc, char* argv[])
{
	RemoteV8::Arguments args(argc, argv);
	boost::lockfree::queue<std::string*, boost::lockfree::fixed_sized<false>> incomingData(100);
	int v8Threads = args.v8threads;
	boost::thread reader(
		boost::bind(
			readerThread, 
			boost::ref(args), 
			boost::ref(incomingData)
		)
	);

	V8::InitializeICU();
	Platform* platform = platform::CreateDefaultPlatform();
	V8::InitializePlatform(platform);
	V8::Initialize();

	boost::thread_group workers;
	for (int i = 0; i < v8Threads; i++)
		workers.create_thread(
			boost::bind(
				workerThread, 
				boost::ref(args), 
				boost::ref(incomingData)
			)
		);

	writerThread(args);
	return 0;
}


void v8ExampleCompute()
{
	// Initialize V8.
	V8::InitializeICU();
	Platform* platform = platform::CreateDefaultPlatform();
	V8::InitializePlatform(platform);
	V8::Initialize();

	v8Extensions::logFilename = std::string("out.txt");


	// Create a new Isolate and make it the current one.
	Isolate* isolate = Isolate::New();
	{
		Isolate::Scope isolate_scope(isolate);

		// Create a stack-allocated handle scope.
		HandleScope handle_scope(isolate);

		Local<ObjectTemplate> global = ObjectTemplate::New(isolate);
		global->Set(String::NewFromUtf8(isolate, "log"),
			FunctionTemplate::New(isolate, v8Extensions::LogCallback));

		// Create a new context.
		Local<Context> context = Context::New(isolate, NULL, global);

		// Enter the context for compiling and running the hello world script.
		Context::Scope context_scope(context);

		// Create a string containing the JavaScript source code.
		Local<String> source = String::NewFromUtf8(isolate, "var a=3;\nlog(Math.pow(a, 0.5));\n");

		// Compile the source code.
		Local<Script> script = Script::Compile(source);

		// Run the script to get the result.
		Local<Value> result = script->Run();

		// Convert the result to an UTF8 string and print it.
		String::Utf8Value utf8(result);
		printf("%s\n", *utf8);
	}

	// Dispose the isolate and tear down V8.
	isolate->Dispose();
	V8::Dispose();
	V8::ShutdownPlatform();
	delete platform;
}


void readerThread(RemoteV8::Arguments &args, 
				  boost::lockfree::queue<std::string*, boost::lockfree::fixed_sized<false>> &incomingData)
{
	auto &stream = args.getCsvDataStream();
	std::string data;
	while (std::getline(stream, data))
	{
		boost::replace_all(data, "\r\n", "");
		incomingData.push(new std::string(data));
	}
		
	args.inputDataStreamEnded = true;
}


void workerThread(RemoteV8::Arguments &args,
				  boost::lockfree::queue<std::string*, boost::lockfree::fixed_sized<false>> &incomingData)
{
	std::string srcipt = args.getScript();
	Isolate* isolate = Isolate::New();
	Isolate::Scope isolate_scope(isolate);
	HandleScope handle_scope(isolate);
	Local<ObjectTemplate> global = ObjectTemplate::New(isolate);
	global->Set(String::NewFromUtf8(isolate, "toOutput"),
		FunctionTemplate::New(isolate, v8Extensions::toOutputCallback));

	Local<Context> context = Context::New(isolate, NULL, global);
	Context::Scope context_scope(context);

	forever 
	{
		std::string *data;
		if (!incomingData.pop(data))
			if (args.inputDataStreamEnded) break;
			else continue;
		std::string src = "params = '";
		src += data[0];
		src += "';\n";
		src += srcipt;
		delete data;

		Local<String> source = String::NewFromUtf8(isolate, src.c_str());
		Local<Script> script = Script::Compile(source);
		Local<Value> result = script->Run();
	}

	args.v8threadsDecrease();
}


void writerThread(RemoteV8::Arguments &args)
{
	std::string *data;
	bool success;
	while (args.v8threads)
	{
		// auto data = v8Extensions::resultingDataQueue.pop();
		success = v8Extensions::outcomingData.pop(data);
		if (success == false)
			if (args.v8threads == 0) break;
			else continue;
		
		//if (!data.empty())
		args.getLogStream() << data[0] << "\n";
		delete data;
	}

	for (int i = 0; i < 1000; i++)
		if (v8Extensions::outcomingData.pop(data)) {
			args.getLogStream() << data[0] << "\n";
			delete data;
		}
}