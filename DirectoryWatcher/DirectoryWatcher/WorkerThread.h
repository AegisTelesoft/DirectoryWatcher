#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <mutex>

#include "DirectoryWatcherStructs.h"

using std::thread;
using std::mutex;

class WorkerThread
{
public:
	WorkerThread(void(&f)(WorkerThreadData), WorkerThreadData data);
	~WorkerThread();
	void ThreadJoin();

private:
	thread m_thread;
};