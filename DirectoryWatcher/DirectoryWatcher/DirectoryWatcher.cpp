#include "DirectoryWatcher.h"

/**************************************************************************************************/
/*                                                                                                */
/**************************************************************************************************/
class SharedThreadData
{
public: 
	CancelationToken& token;
	dw_callback callback;
	bool watchSubtree; 
public:
	SharedThreadData(CancelationToken& token, 
		dw_callback callback, bool watchSubtree);
};

/**************************************************************************************************/
/*                                                                                                */
/**************************************************************************************************/
class MasterThreadData
{
public:
	vector<string>&  directories;
	vector<string>&  newDirectories;
	vector<string>&  dirsToRemove;
	mutex&           addRemoveMutex;
	SharedThreadData sharedData;
public:
	MasterThreadData(vector<string>& directories, vector<string>& newDirectories, 
		vector<string>& dirsToRemove, mutex& dwMutex, SharedThreadData sharedData);
};

/**************************************************************************************************/
/*                                                                                                */
/**************************************************************************************************/
class UniqueWorkerTreadData
{
public:
	int threadId;
	string directory;
public:
	UniqueWorkerTreadData(int threadId, string directory);
};

void masterThreadTask(MasterThreadData data);
typedef pair<UniqueWorkerTreadData, SharedThreadData> worker_data;
typedef pair<thread, UniqueWorkerTreadData> thread_pair;

/**************************************************************************************************/
/*                                                                                                */
/**************************************************************************************************/
DirectoryWatcher::DirectoryWatcher(dw_callback callback) 
	: m_callback(callback), m_isWatching(false) {
	
}

/**************************************************************************************************/
DirectoryWatcher::DirectoryWatcher(string& directory, dw_callback callback) 
	: m_callback(callback), m_isWatching(false) {
	m_directories.push_back(directory);
}

/**************************************************************************************************/
DirectoryWatcher::DirectoryWatcher(vector<string>& directories, dw_callback callback) 
	: m_callback(callback), m_isWatching(false) {
	for (int i = 0; i < directories.size(); i++) 
	{
		m_directories.push_back(directories[i]);
	}
}

/**************************************************************************************************/
bool DirectoryWatcher::Watch(bool watchSubDir)
{
	if (!m_isWatching && m_directories.size() > 0)
	{
		m_ct.ResetGlobalToken();
		m_ct.ResetIdToken();
		SharedThreadData sharedData(m_ct, m_callback, watchSubDir);
		m_masterThread = thread(masterThreadTask, MasterThreadData(m_directories, m_newDirectories,
			m_dirsToRemove, m_mutex, sharedData));

		m_isWatching = true;
		return m_isWatching;
	}
   	return false;
}

/**************************************************************************************************/
void DirectoryWatcher::Stop() 
{
	if (m_isWatching) 
	{
		m_ct.CancelGlobally();
		m_masterThread.join();
		m_isWatching = false;
	}
}   

/**************************************************************************************************/
void DirectoryWatcher::AddDir(string& directory)
{
	std::unique_lock<std::mutex> lock(m_mutex);
	m_directories.push_back(directory);
	m_newDirectories.push_back(directory);
	lock.unlock();
}

/**************************************************************************************************/
void DirectoryWatcher::RemoveDir(string& directory)
{
	std::unique_lock<std::mutex> lock(m_mutex);
	m_dirsToRemove.push_back(directory);
	lock.unlock();
}

/**************************************************************************************************/
DirectoryWatcher::~DirectoryWatcher() 
{
	if (m_isWatching) 
	{
		m_ct.CancelGlobally();
		m_masterThread.join();
	}
}

/**************************************************************************************************/
/*                                                                                                */
/**************************************************************************************************/
int watchDirectory(worker_data data)
{
	CHAR buffer[8192];
	CHAR lpDir[_MAX_DIR];
	DWORD BytesReturned;
	DWORD watcherWaitStatus;
	HANDLE dwChangesEventHandle;
	const DWORD FILE_NOTIFY_EVERYTHING = 
		  FILE_NOTIFY_CHANGE_DIR_NAME	 | FILE_NOTIFY_CHANGE_FILE_NAME  
		| FILE_NOTIFY_CHANGE_LAST_WRITE;


	const char* path = data.first.directory.c_str();
	_splitpath_s(path, NULL, 0, lpDir, _MAX_DIR, NULL, 0, NULL, 0);

	dwChangesEventHandle = FindFirstChangeNotificationA(lpDir, data.second.watchSubtree,
		FILE_NOTIFY_EVERYTHING);

	if (dwChangesEventHandle == INVALID_HANDLE_VALUE)
	{
		cout << "\nFailed to watch dir for " << lpDir <<  endl;
		return -1;
	}


	while (!data.second.token.IsGloballyCanceled())
	{
		if (data.second.token.IsCanceled(data.first.threadId))
		{
			data.second.token.ResetIdToken();
			break;
		}

		watcherWaitStatus = WaitForSingleObject(dwChangesEventHandle, 100);
		
		switch (watcherWaitStatus)
		{
		case WAIT_OBJECT_0:

			if (ReadDirectoryChangesW(dwChangesEventHandle, buffer, sizeof(buffer),
				data.second.watchSubtree, FILE_NOTIFY_EVERYTHING, &BytesReturned, NULL, NULL))
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
					action = RenamedFrom;
					break;
				case FILE_ACTION_RENAMED_NEW_NAME:
					action = RenamedTo;
					break;
				}

				data.second.callback(fileName, action);
			}

			if (FindNextChangeNotification(dwChangesEventHandle) == FALSE)
			{
				cout << "\nFailed to watch directory for file creation and deletion" << endl;
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
	return 0;
}

/**************************************************************************************************/
/*                                                                                                */
/**************************************************************************************************/
void masterThreadTask(MasterThreadData data)
{

	vector<thread_pair> workers;

	for(int i = 0; i < data.directories.size(); i++)
	{
		UniqueWorkerTreadData uniqueThreadData(workers.size(), data.directories[i]);
		worker_data wtData(make_pair(uniqueThreadData, data.sharedData));
		thread_pair workerDataPair(make_pair(thread(watchDirectory, move(wtData)), 
			uniqueThreadData));
		workers.push_back(move(workerDataPair));
	}

	while (!data.sharedData.token.IsGloballyCanceled())
	{
		unique_lock<mutex> lock(data.addRemoveMutex);
		if (data.newDirectories.size() > 0)
		{
			for (int i = data.newDirectories.size() - 1; i >= 0; i--)
			{
				UniqueWorkerTreadData uniqueThreadData(workers.size(), data.newDirectories[i]);
				worker_data wtData(make_pair(uniqueThreadData, data.sharedData));
				thread_pair workerDataPair(make_pair(thread(watchDirectory, move(wtData)), 
					uniqueThreadData));
				workers.push_back(move(workerDataPair));

				data.newDirectories.erase(data.newDirectories.begin() + i);
			}
		}

 		if (data.dirsToRemove.size() > 0)
		{
			for (int i = data.dirsToRemove.size() - 1; i >= 0; i--)
			{
				auto result = std::find_if(workers.begin(), workers.end(), 
					[data, i](const thread_pair& element) {
						return element.second.directory == data.directories[i];
					});

				if (result != std::end(workers)) {
					int workerId = std::distance(workers.begin(), result);
					if (!data.sharedData.token.Cancel(workerId))
						break;
				}
				data.dirsToRemove.erase(data.dirsToRemove.begin() + i);
			}
		}
		lock.unlock();
		std::this_thread::sleep_for(std::chrono::milliseconds(200));
	}

	for (int i = 0; i < workers.size(); i++)
	{
		workers[i].first.join();
	}
}

/**************************************************************************************************/
/*                                                                                                */
/**************************************************************************************************/
MasterThreadData::MasterThreadData(vector<string>& directories, vector<string>& newDirectories, 
	vector<string>& dirsToRemove, mutex& dwMutex, SharedThreadData sharedData)
		: directories(directories), newDirectories(newDirectories), dirsToRemove(dirsToRemove), 
		addRemoveMutex(dwMutex), sharedData(sharedData) { }

/**************************************************************************************************/
/*                                                                                                */
/**************************************************************************************************/
SharedThreadData::SharedThreadData(CancelationToken& token, dw_callback callback, bool watchSubtree)
	: token(token), callback(callback), watchSubtree(watchSubtree) { }

/**************************************************************************************************/
/*                                                                                                */
/**************************************************************************************************/
UniqueWorkerTreadData::UniqueWorkerTreadData(int threadId, string directory)
	: threadId(threadId), directory(directory) { }