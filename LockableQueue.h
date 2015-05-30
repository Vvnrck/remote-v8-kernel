#ifndef LOCKABLE_QUEUE
#define LOCKABLE_QUEUE


#include "stdafx.h"

template<typename T>
class LockableQueue
{
private:
	std::queue<T> queue;
	boost::mutex mutex;

public:
	LockableQueue() : queue(), mutex() {};

	void push(T item)
	{
		mutex.lock();
		queue.push(item);
		mutex.unlock();
	}

	T pop()
	{
		T item;
		mutex.lock();
		if (!queue.empty())
		{
			item = queue.front();
			queue.pop();
		}
			
		mutex.unlock();
		return item;
	}

	bool empty()
	{
		mutex.lock();
		bool e = queue.empty();
		mutex.unlock();
		return e;
	}
};

#endif