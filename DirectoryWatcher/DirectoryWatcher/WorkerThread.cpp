#include "WorkerThread.h";

WorkerThread::WorkerThread(void(&f)(struct WorkerThreadData), struct WorkerThreadData data)
	: m_thread(thread(f, data))
{

}

void WorkerThread::ThreadJoin()
{
	m_thread.join();
}

WorkerThread::~WorkerThread()
{

}