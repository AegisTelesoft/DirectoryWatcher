#include "DirectoryWatcher.h"

#include <iostream>
#include <mutex>
#include <chrono>
#include <algorithm>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>

#if defined (WINDOWS)
	#include <windows.h>
	#include <tchar.h>
#elif defined (LINUX)
	#include <dirent.h>
#endif

using std::vector;
using std::thread;
using std::string;
using std::mutex;
using std::unique_lock;
using std::make_pair;
using std::pair;
using std::move;
using std::cout;
using std::endl;

typedef pair<string, struct stat> file;

/**************************************************************************************************/
/*          ThreadData class object is being passed to DirectoryWatcher worker thread             */
/**************************************************************************************************/
class ThreadData
{
public:
	int ThreadId;
	std::string Directory;
	CancelationToken& Token;
	StatusFlag<DirectoryWatcherStatus>& Status;
	dw_callback Callback;
	bool WatchSubtree;
public:
	ThreadData(int threadId, std::string directory, CancelationToken& token, 
		StatusFlag<DirectoryWatcherStatus>& status, dw_callback callback, bool watchSubtree);
};

/**************************************************************************************************/
namespace Utilities
{
#if defined (WINDOWS)

	string GetErrorMessage()
	{
		DWORD errorMessageID = ::GetLastError();
		if (errorMessageID == 0)
			return "";

		LPSTR messageBuffer = nullptr;
		size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

		string error(messageBuffer, size);
		LocalFree(messageBuffer);
		return error;
	}

	/**************************************************************************************************/
	static vector<file> findAllFilesAndDirs(string dir, bool watchSubDirs)
	{
		vector<file> results;

		HANDLE findHandle;
		WIN32_FIND_DATAA fileInfo;

		CHAR dirPath[_MAX_DIR];
		_splitpath_s(dir.c_str(), NULL, 0, dirPath, _MAX_DIR, NULL, 0, NULL, 0);
		std::strncat(dirPath, "*.*", _MAX_DIR - 1);

		findHandle = FindFirstFileA(dirPath, &fileInfo);

		if (findHandle != INVALID_HANDLE_VALUE)
		{

			struct stat dirStatBuffer;
			stat(dir.c_str(), &dirStatBuffer);
			results.push_back(move(make_pair(string(dir), move(dirStatBuffer))));

			do
			{
				string fullPath = (string(dir) + string(fileInfo.cFileName));

				struct stat fileStatBuffer;
				stat(fullPath.c_str(), &fileStatBuffer);

				// Check if file is a directory
				if (fileInfo.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				{
					char key1[] = ".";
					char key2[] = "..";

					// check if folders aren't strange dots (probably path to forward and back)
					if (strcmp(fileInfo.cFileName, key1) && strcmp(fileInfo.cFileName, key2))
					{

						if (watchSubDirs)
						{
							vector<file> result = findAllFilesAndDirs(fullPath + "\\", watchSubDirs);

							for (int i = 0; i < result.size(); i++)
								results.push_back(move(result[i]));
						}
						else
						{
							results.push_back(move(make_pair(fullPath+ "\\", move(fileStatBuffer))));
						}

					}
				}
				else
				{
					results.push_back(move(make_pair(fullPath, move(fileStatBuffer))));
				}

			} while (FindNextFileA(findHandle, &fileInfo));
			FindClose(findHandle);
		}
		else
			cout << Utilities::GetErrorMessage() << endl;

		return move(results);
	}

#elif defined (LINUX)

	static vector<file> findAllFilesAndDirs(string directory, bool watchSubDirs)
	{
		vector<file> results;

		string dirToOpen = directory;
		auto dir = opendir(dirToOpen.c_str());

		if(dir != NULL)
		{
			struct stat fileStatBuffer;
			stat(directory.c_str(), &fileStatBuffer);
			results.push_back(move(make_pair(directory, move(fileStatBuffer))));

			auto entity = readdir(dir);

			while(entity != NULL)
			{
				if(entity->d_type == DT_DIR)
				{
					if(entity->d_name[0] != '.')
					{
						string fullDirPath = directory + string(entity->d_name) + "/";

						if (watchSubDirs)
						{
							vector<file> result = findAllFilesAndDirs(fullDirPath, watchSubDirs);

							for (int i = 0; i < result.size(); i++)
								results.push_back(move(result[i]));
						}
						else
							results.push_back(move(make_pair(fullDirPath, move(fileStatBuffer))));
					}
				}

				if(entity->d_type == DT_REG)
				{
					string fullFilePath = directory + string(entity->d_name);

					struct stat fileStatBuffer;
					stat(fullFilePath.c_str(), &fileStatBuffer);

					results.push_back(move(make_pair(fullFilePath, move(fileStatBuffer))));
				}

				entity = readdir(dir);
			}
		}
		closedir(dir);

		return move(results);
	}

#endif 
}

/**************************************************************************************************/
/*                    Separated worker thread functions to limit usage scope                      */
/**************************************************************************************************/
namespace WorkerTasks 
{
	static void CheckOldTree(vector<file>& tree, dw_callback callback)
	{
		for (int i = 0; i < tree.size(); i++)
		{
			struct stat tmpBuffer;

			if (stat(tree[i].first.c_str(), &tmpBuffer) == -1)
			{
				callback(tree[i].first, Deleted, string("Deleted"));
			}
			else if (difftime(tmpBuffer.st_mtime, tree[i].second.st_mtime) != 0 && tmpBuffer.st_size != tree[i].second.st_size)
			{
				// File size changed or it was modified
				callback(tree[i].first, Modified, string("Modified Content"));

				tree[i].second = tmpBuffer;
			}
		}
	}

	/**************************************************************************************************/
	static void CheckNewTree(vector<file>& oldTree, vector<file>& newTree, dw_callback callback)
	{
		// Check if new files were created
		for (int i = 0; i < newTree.size(); i++)
		{
			auto result = std::find_if(oldTree.begin(), oldTree.end(),
				[newTree, i](const file& element) {
				return element.first == newTree[i].first;
			});

			if (result == std::end(oldTree))
				callback(newTree[i].first, Added, string("Added"));
		}
	}

	/**************************************************************************************************/
	/* Worker thread tasks that mainly checks old directory and file tree for modifications, creates  */
	/* new directory and file tree and checks for new files/folders                                           */
	/**************************************************************************************************/
	static int watchDirectory(ThreadData data)
	{
		vector<file> tree = Utilities::findAllFilesAndDirs(data.Directory, data.WatchSubtree);

		data.Status.SetStatus(Watching);

		// Checking if threads aren't canceled globally
		while (!data.Token.IsGloballyCanceled())
		{
			// Checking if this thread isn't canceled due to DirectoryWatcher.Remove(...) method
			if (data.Token.IsCanceled(data.ThreadId)) //Check for reset
			{
				data.Token.ResetIdToken();
				return 0;
			}

			// Checking old directory and file tree
			WorkerTasks::CheckOldTree(tree, data.Callback);

			vector<file> newTree = Utilities::findAllFilesAndDirs(data.Directory, data.WatchSubtree);

			// Checking new directory and file tree
			WorkerTasks::CheckNewTree(tree, newTree, data.Callback);

			tree = move(newTree);

			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}
		return 0;
	}
}


/**************************************************************************************************/
/*                                                                                                */
/**************************************************************************************************/
DirectoryWatcher::DirectoryWatcher(dw_callback callback) 
	: m_callback(callback), m_isWatching(false), m_statusFlag(Idle) {
	
}

/**************************************************************************************************/
DirectoryWatcher::DirectoryWatcher(string directory, dw_callback callback) 
	: m_callback(callback), m_isWatching(false), m_statusFlag(Idle) {
	m_directories.push_back(directory);
}

/**************************************************************************************************/
DirectoryWatcher::DirectoryWatcher(vector<string> directories, dw_callback callback) 
	: m_callback(callback), m_isWatching(false), m_statusFlag(Idle) {
	for (int i = 0; i < directories.size(); i++) 
	{
		m_directories.push_back(directories[i]);
	}
}

/**************************************************************************************************/
/* This method starts directory watcher. This method creates thread for each given directory      */
/**************************************************************************************************/
bool DirectoryWatcher::Watch(bool watchSubDir)
{
	if (!m_isWatching)
	{
		m_watchSubDiretories = watchSubDir;
		m_ct.ResetGlobalToken();
		m_ct.ResetIdToken();

		for (int i = 0; i < m_directories.size(); i++)
		{
			ThreadData data(m_watchers.size(), m_directories[i], m_ct, m_statusFlag, m_callback, m_watchSubDiretories);
			watcher workerDataPair(make_pair(thread(WorkerTasks::watchDirectory, data), m_directories[i]));
			m_watchers.push_back(move(workerDataPair));
		}

		m_isWatching = true;
		return m_isWatching;
	}
   	return false;
}

/**************************************************************************************************/
/*            This method cancels all worker threads via custom cancelation token                 */
/**************************************************************************************************/
void DirectoryWatcher::Stop() 
{
	if (m_isWatching) 
	{
		m_ct.CancelGlobally();
		for (int i = 0; i < m_watchers.size(); i++)
		{
			m_watchers[i].first.join();
		}
		m_isWatching = false;
		m_statusFlag.SetStatus(Idle);
	}
}   

/**************************************************************************************************/
/*  This method adds directory to watching list. In case DirectoryWatcher is already running this */
/*  methods starts new thread that watches given directory                                        */
/**************************************************************************************************/
void DirectoryWatcher::AddDir(string directory)
{
	if (m_isWatching)
	{
		ThreadData data(m_watchers.size(), directory, m_ct, m_statusFlag, m_callback, m_watchSubDiretories);
		watcher workerDataPair(make_pair(thread(WorkerTasks::watchDirectory, data), directory));
		m_watchers.push_back(move(workerDataPair));
	}
	m_directories.push_back(directory);
}

/**************************************************************************************************/
/* This method stops worker thread that watches given directory and removes that directory from   */
/* directory watcher's list																		  */
/**************************************************************************************************/
void DirectoryWatcher::RemoveDir(string directory)
{
	auto workerIterator = std::find_if(m_watchers.begin(), m_watchers.end(),
		[directory](const watcher& element) {
		return element.second == directory;
	});
		
	if (workerIterator != std::end(m_watchers))
	{
		int workerId = std::distance(m_watchers.begin(), workerIterator);

		while (!m_ct.Cancel(workerId));
	}

	auto dirIterator = std::find_if(m_directories.begin(), m_directories.end(),
		[directory](const string& element) {
		return element == directory;
	});

	if (dirIterator != std::end(m_directories))
		m_directories.erase(dirIterator);
	
}

/**************************************************************************************************/
DirectoryWatcherStatus DirectoryWatcher::GetStatus()
{
	return m_statusFlag.GetStatus();
}

/**************************************************************************************************/
DirectoryWatcher::~DirectoryWatcher() 
{
	if (m_isWatching) 
	{
		m_ct.CancelGlobally();
		for (int i = 0; i < m_watchers.size(); i++)
		{
			m_watchers[i].first.join();
		}
	}
}

/**************************************************************************************************/
/*                                                                                                */
/**************************************************************************************************/
ThreadData::ThreadData(int threadId, string directory, CancelationToken& token, StatusFlag<DirectoryWatcherStatus>& status, dw_callback callback, bool watchSubtree)
	: ThreadId(threadId), Directory(directory), Token(token), Status(status), Callback(callback), WatchSubtree(watchSubtree) { }
