#include "DirectoryWatcher.h"

DirectoryWatcher::DirectoryWatcher()
{
	
}

DirectoryWatcher::DirectoryWatcher(string& directory)
{
	m_directories.push_back(Directory(directory, 0));
}

DirectoryWatcher::DirectoryWatcher(vector<string>& directories) 
{
	for (int i = 0; i < directories.size(); i++) 
	{
		m_directories.push_back(Directory(directories[i], i));
	}
}

void DirectoryWatcher::Watch(bool watchSubDir)
{
	if (!m_isWatching) 
	{
		m_masterThread = thread(masterThreadTask, struct MasterThreadData(m_directories, m_ctPtr));
		m_isWatching = true;
	}
}

void masterThreadTask(struct MasterThreadData data)
{
	vector<thread> workers;

	for (int i = 0; i < data.directories.size(); i++)
	{
		workers.push_back(thread(workerThreadTask, struct WorkerThreadData(data.directories[i], i, data.token)));
		
	}

	/*while (!data.token->IsCanceled())
	{
		
	}*/

	for (int i = 0; i < workers.size(); i++)
	{
		workers[i].join();
	}
}

void workerThreadTask(struct WorkerThreadData data)
{
	std::cout << "Slave nr.: " << data.threadId << std::endl;
	while (true) 
	{
	
		// doing some work here
		std::this_thread::sleep_for(std::chrono::milliseconds(5000));
		std::cout << "Work work.... " << std::endl;
		
		if (data.token->IsCanceled())
		{
			std::cout << "Canceled thread! " << data.threadId << " " << std::endl;
			break;
		}
	}
}

void DirectoryWatcher::AddDirectory(string& directory)
{

}

void DirectoryWatcher::RemoveDirectory(int id) 
{

}

void DirectoryWatcher::RemoveDirectory(string& directory)
{

}

void DirectoryWatcher::Stop() 
{
	m_ctPtr->Cancel();
	m_isWatching = false;
}   

DirectoryWatcher::~DirectoryWatcher() 
{
	m_ctPtr->Cancel();
	m_masterThread.join();
}

Directory::Directory(string path, int id)
	: Path(path), Id(id) { 
}

MasterThreadData::MasterThreadData(vector<Directory> &directories, CancelationToken* token)
	: directories(directories), token(token) {
}

WorkerThreadData::WorkerThreadData(Directory dir, int threadId, CancelationToken* token)
	: directory(dir), threadId(threadId), token(token) {
}
