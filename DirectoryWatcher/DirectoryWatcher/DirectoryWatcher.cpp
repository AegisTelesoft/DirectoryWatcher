#include "DirectoryWatcher.h"

DirectoryWatcher::DirectoryWatcher()
{
	
}

DirectoryWatcher::DirectoryWatcher(string& directory)
{
	m_newDirectories.push_back(string(directory));
}

DirectoryWatcher::DirectoryWatcher(vector<string>& directories) 
{
	for (int i = 0; i < directories.size(); i++) 
	{
		m_newDirectories.push_back(string(directories[i]));
	}
}

void DirectoryWatcher::Watch(bool watchSubDir)
{
	if (!m_isWatching) 
	{
		m_masterThread = thread(masterThreadTask, MasterThreadData(&m_newDirectories, &m_dirsToRemove, &m_ct));
		m_isWatching = true;
	}
}

void masterThreadTask(MasterThreadData data)
{
	vector<std::pair<thread, WorkerThreadData>> workers;

	while (!data.token->IsGloballyCanceled())
	{
		// Master thread checks if there are any new directories added
		if (data.newDirectories->size() > 0)
		{
			for (int i = data.newDirectories->size() - 1; i >= 0; i--)
			{
				WorkerThreadData workerData(data.newDirectories->operator[](i), workers.size() + 1, data.token);
				workers.push_back(std::make_pair(thread(workerThreadTask, workerData), workerData));
				data.newDirectories->erase(data.newDirectories->begin() + i);
			}
		}

		// Master thread check if there are any new directories to remove
 		if (data.dirsToRemove->size() > 0)
		{
			for (int i = data.dirsToRemove->size() - 1; i >= 0; i--)
			{
				auto result = std::find_if(workers.begin(), workers.end(), 
					[data, i](const std::pair<thread, WorkerThreadData>& element) {
						return element.second.directory == data.dirsToRemove->operator[](i);
					});

				if (result != std::end(workers)) {

					//Cancel the thread
					int workerId = std::distance(workers.begin(), result);
					if (!data.token->Cancel(workerId))
						break;
				}
				data.dirsToRemove->erase(data.dirsToRemove->begin() + i);
			}
		}
	}

	for (int i = 0; i < workers.size(); i++)
	{
		workers[i].first.join();
	}
}

void workerThreadTask(WorkerThreadData data)
{
	std::cout << "Slave nr.: " << data.threadId << std::endl;
	while (true) 
	{
	
		// doing some work here
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		
		if (data.token->IsGloballyCanceled() || data.token->IsCanceled(data.threadId))
		{
			if (data.token->IsCanceled(data.threadId))
				data.token->Reset();
			std::cout << "Canceled thread! " << data.threadId << " " << std::endl;
			break;
		}
	}
}

void DirectoryWatcher::AddDirectory(string& directory)
{
	m_newDirectories.push_back(string(directory));
}

void DirectoryWatcher::RemoveDirectory(string& directory)
{
	m_dirsToRemove.push_back(directory);
}

void DirectoryWatcher::Stop() 
{
	m_ct.CancelGlobally();
	m_isWatching = false;
}   

DirectoryWatcher::~DirectoryWatcher() 
{
	m_ct.CancelGlobally();
	m_masterThread.join();
}

MasterThreadData::MasterThreadData(vector<string>* directories, vector<string>* dirsToRemove, CancelationToken* token)
	: newDirectories(directories), dirsToRemove(dirsToRemove), token(token) {
}

WorkerThreadData::WorkerThreadData(string dir, int threadId, CancelationToken* token)
	: directory(dir), threadId(threadId), token(token) {
}
