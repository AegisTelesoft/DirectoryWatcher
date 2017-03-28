#include "DirectoryWatcher.h"

DirectoryWatcher::DirectoryWatcher(dw_callback callback) : m_callback(callback)
{
	
}

DirectoryWatcher::DirectoryWatcher(string& directory, dw_callback callback) : m_callback(callback)
{
	m_newDirectories.push_back(string(directory));
}

DirectoryWatcher::DirectoryWatcher(vector<string>& directories, dw_callback callback) : m_callback(callback)
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
		m_masterThread = thread(masterThreadTask, MasterThreadData(&m_newDirectories, &m_dirsToRemove, &m_ct, m_callback));
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
				WorkerThreadData workerData(data.newDirectories->operator[](i), workers.size() + 1, data.token, data.callback);
				workers.push_back(std::make_pair(thread(watchDirectory, workerData), workerData));
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

//void workerThreadTask(WorkerThreadData data)
//{
//	cout << "Slave nr.: " << data.threadId << endl;
//	while (true) 
//	{
//	
//
//		
//		if (data.token->IsGloballyCanceled() || data.token->IsCanceled(data.threadId))
//		{
//			if (data.token->IsCanceled(data.threadId))
//				data.token->Reset();
//			cout << "Canceled thread! " << data.threadId << " " << endl;
//			break;
//		}
//	}
//}

int watchDirectory(WorkerThreadData data)
{
#ifdef _WIN32
	DWORD watcherWaitStatus;
	HANDLE watcherEventHandles[2];
	CHAR lpDir[_MAX_DIR];
	CHAR lpFile[_MAX_FNAME];
	CHAR lpExt[_MAX_EXT];

	const char* path = data.directory.c_str();

	_splitpath_s(path, NULL, 0, lpDir, _MAX_DIR, lpFile, _MAX_FNAME, lpExt, _MAX_EXT);

	watcherEventHandles[0] = FindFirstChangeNotificationA(lpDir, TRUE, FILE_NOTIFY_CHANGE_FILE_NAME);
	watcherEventHandles[1] = FindFirstChangeNotificationA(lpDir, TRUE, FILE_NOTIFY_CHANGE_DIR_NAME);

	if (watcherEventHandles[0] == INVALID_HANDLE_VALUE || watcherEventHandles[1] == INVALID_HANDLE_VALUE)
	{
		cout << "\nFailed to Directory watcher for " << lpDir <<  endl;
		return -1;
	}


	while (!data.token->IsGloballyCanceled())
	{
		if (data.token->IsCanceled(data.threadId))
		{
			data.token->Reset();
			break;
		}

		watcherWaitStatus = WaitForMultipleObjects(2, watcherEventHandles, FALSE, 1000);

		switch (watcherWaitStatus)
		{
		case WAIT_OBJECT_0:

			//cout << "\nA file was created, renamed, or deleted in the directory" << endl;
			data.callback(string(lpDir), FileNameChanged);

			if (FindNextChangeNotification(watcherEventHandles[0]) == FALSE)
			{
				cout << "\nFailed to watch directory for file creation and deletion" << endl;
				return -1;
			}
			break;

		case WAIT_OBJECT_0 + 1:

			//cout << "\nA directory was created, renamed, or deleted." << endl;
			data.callback(string(lpDir), DirectoryNameChanged);

			if (FindNextChangeNotification(watcherEventHandles[1]) == FALSE)
			{
				cout << "\nFailed to watch directory for directory creation and deletion" << endl;
				return -1;
			}
			break;

		case WAIT_TIMEOUT:

			break;

		default:
			cout << "\nUnhandled DirectoryWatcher wait status." << endl;
			return -1;
			break;
		}
	}
#endif
	return -1;
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

MasterThreadData::MasterThreadData(vector<string>* directories, vector<string>* dirsToRemove, CancelationToken* token, dw_callback callback)
	: newDirectories(directories), dirsToRemove(dirsToRemove), token(token), callback(callback){
}

WorkerThreadData::WorkerThreadData(string dir, int threadId, CancelationToken* token, dw_callback callback)
	: directory(dir), threadId(threadId), token(token), callback(callback) {
}
