#include "DirectoryWatcher.h"

DirectoryWatcher::DirectoryWatcher(dw_callback callback) : m_callback(callback), m_isWatching(false)
{
	
}

DirectoryWatcher::DirectoryWatcher(string& directory, dw_callback callback) : m_callback(callback), m_isWatching(false)
{
	m_newDirectories.push_back(string(directory));
}

DirectoryWatcher::DirectoryWatcher(vector<string>& directories, dw_callback callback) : m_callback(callback), m_isWatching(false)
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
		m_masterThread = thread(masterThreadTask, MasterThreadData(&m_newDirectories, watchSubDir, &m_dirsToRemove, &m_ct, m_callback));
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
				WorkerThreadData workerData(data.newDirectories->operator[](i), data.watchSubtree, workers.size(), data.token, data.callback);
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

int watchDirectory(WorkerThreadData data)
{
	DWORD BytesReturned = 0;
	CHAR buffer[2048];
	OVERLAPPED overlapped;
	const DWORD FILE_NOTIFY_EVERYTHING = 
		FILE_NOTIFY_CHANGE_ATTRIBUTES | FILE_NOTIFY_CHANGE_DIR_NAME 
		| FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_LAST_WRITE 
		| FILE_NOTIFY_CHANGE_SECURITY | FILE_NOTIFY_CHANGE_SIZE;

	DWORD watcherWaitStatus;
	HANDLE dwChangesEventHandle;
	CHAR lpDir[_MAX_DIR];
	CHAR lpFile[_MAX_FNAME];
	CHAR lpExt[_MAX_EXT];

	const char* path = data.directory.c_str();
	_splitpath_s(path, NULL, 0, lpDir, _MAX_DIR, lpFile, _MAX_FNAME, lpExt, _MAX_EXT);

	dwChangesEventHandle = FindFirstChangeNotificationA(lpDir, data.watchSubtree, FILE_NOTIFY_EVERYTHING);

	if (dwChangesEventHandle == INVALID_HANDLE_VALUE)
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

		watcherWaitStatus = WaitForSingleObjectEx(dwChangesEventHandle, 1000, TRUE);
		
		switch (watcherWaitStatus)
		{
		case WAIT_OBJECT_0:

			if (ReadDirectoryChangesW(dwChangesEventHandle, buffer, 2048, data.watchSubtree, FILE_NOTIFY_EVERYTHING, &BytesReturned, &overlapped, &ReadDirChangesCallback))
			{
				FILE_NOTIFY_INFORMATION* notifInfo = (FILE_NOTIFY_INFORMATION*)(buffer);
				_bstr_t bstr(notifInfo->FileName);
 				string fileName = string(bstr).substr(0, notifInfo->FileNameLength / sizeof(WCHAR));
				
				ChangeType action;

				switch (notifInfo->Action)
				{
				case FILE_ACTION_ADDED:
					action = Added;
					break;
				case FILE_ACTION_REMOVED:
					action = Deleted;
					break;
				case FILE_ACTION_MODIFIED:
					action = Modified;
					break;
				case FILE_ACTION_RENAMED_OLD_NAME:
					action = Renamed; 
					break;
				case FILE_ACTION_RENAMED_NEW_NAME:
					action = Renamed;
					break;
				}
				
				data.callback(fileName, action);
			}

			if (FindNextChangeNotification(dwChangesEventHandle) == FALSE)
			{
				cout << "\nFailed to watch directory for file creation and deletion" << endl;
				return -1;
			}
			break;

		case WAIT_IO_COMPLETION:
			//  IT's happening
			break;

		case WAIT_TIMEOUT:

			break;

		default:
			cout << "\nUnhandled DirectoryWatcher wait status." << endl;
			return -1;
			break;
		}
	}
	return -1;
}

void DirectoryWatcher::AddDir(string& directory)
{
	m_newDirectories.push_back(string(directory));
}

void DirectoryWatcher::RemoveDir(string& directory)
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

MasterThreadData::MasterThreadData(vector<string>* directories, bool watchSubtree, vector<string>* dirsToRemove, CancelationToken* token, dw_callback callback)
	: newDirectories(directories), watchSubtree(watchSubtree), dirsToRemove(dirsToRemove), token(token), callback(callback){
}

WorkerThreadData::WorkerThreadData(string dir, bool watchSubtree, int threadId, CancelationToken* token, dw_callback callback)
	: directory(dir), watchSubtree(watchSubtree), threadId(threadId), token(token), callback(callback) {
}


VOID CALLBACK ReadDirChangesCallback(DWORD errorCode, DWORD bytesTransfered, LPOVERLAPPED lpOverlapped) {
	// break point here don't trigger
	cout << "\n" << bytesTransfered << endl;

}
