#include "DirectoryWatcher.h"

#include <iostream>
#include <sys/stat.h>
#include <chrono>
#include <mutex>
#include <algorithm>

// stat headers 
#include <time.h>
#include <sys/types.h>

//Win api headers
#ifdef _WIN32

#include <windows.h>
#include <tchar.h>
#include <comdef.h>
#include <Strsafe.h>
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
/*                                                                                                */
/**************************************************************************************************/
class ThreadData
{
public:
	int ThreadId;
	std::string Directory;
	CancelationToken& Token;
	dw_callback Callback;
	bool WatchSubtree;
public:
	ThreadData(int threadId, std::string directory, CancelationToken& token, 
		dw_callback callback, bool watchSubtree);
};
/**************************************************************************************************/
/*                                                                                                */
/**************************************************************************************************/

namespace Utilities
{
#ifdef _WIN32

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
	static vector<file> findAllFilesAndDirs(string dir, string rootDir, bool watchSubDirs)
	{
		vector<file> results;
		vector<file> directories;


		HANDLE findHandle;
		WIN32_FIND_DATAA fileInfo;

		CHAR dirPath[_MAX_DIR];
		_splitpath_s(dir.c_str(), NULL, 0, dirPath, _MAX_DIR, NULL, 0, NULL, 0);
		StringCchCatA(dirPath, _MAX_DIR, "*.*");

		findHandle = FindFirstFileA(dirPath, &fileInfo);

		if (findHandle != INVALID_HANDLE_VALUE)
		{
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
						directories.push_back(move(make_pair(fullPath, move(fileStatBuffer))));
				}
				else
					results.push_back(move(make_pair(fullPath, move(fileStatBuffer))));

			} while (FindNextFileA(findHandle, &fileInfo));
			FindClose(findHandle);
		}
		else
			cout << Utilities::GetErrorMessage() << endl;

		if (watchSubDirs)
			for (int i = 0; i < directories.size(); i++)
			{
				vector<file> result = findAllFilesAndDirs(directories[i].first + "\\", rootDir, watchSubDirs);

				for (int i = 0; i < result.size(); i++)
					results.push_back(move(result[i]));
			}

		if (dir.compare(rootDir) == 0)
		{
			struct stat fileStatBuffer;
			stat(rootDir.c_str(), &fileStatBuffer);
			results.push_back(move(make_pair(string(rootDir), move(fileStatBuffer))));
		}

		for (int i = 0; i < directories.size(); i++)
			results.push_back(move(directories[i]));

		return move(results);
	}
#endif // _WIN32
}


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
}

/**************************************************************************************************/
/*                                                                                                */
/**************************************************************************************************/
static int watchDirectory(ThreadData data)
{
	vector<file> tree = Utilities::findAllFilesAndDirs(data.Directory, data.Directory, data.WatchSubtree);

	while (!data.Token.IsGloballyCanceled())
	{
		if (data.Token.IsCanceled(data.ThreadId)) //Check for reset
		{
			data.Token.ResetIdToken();
			return 0;
		}

		WorkerTasks::CheckOldTree(tree, data.Callback);

		vector<file> newTree = Utilities::findAllFilesAndDirs(data.Directory, data.Directory, data.WatchSubtree);

		WorkerTasks::CheckNewTree(tree, newTree, data.Callback);

		tree = move(newTree);

		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
	return 0;
}

/**************************************************************************************************/
/*                                                                                                */
/**************************************************************************************************/
DirectoryWatcher::DirectoryWatcher(dw_callback callback) 
	: m_callback(callback), m_isWatching(false) {
	
}

/**************************************************************************************************/
DirectoryWatcher::DirectoryWatcher(string directory, dw_callback callback) 
	: m_callback(callback), m_isWatching(false) {
	m_directories.push_back(directory);
}

/**************************************************************************************************/
DirectoryWatcher::DirectoryWatcher(vector<string> directories, dw_callback callback) 
	: m_callback(callback), m_isWatching(false) {
	for (int i = 0; i < directories.size(); i++) 
	{
		m_directories.push_back(directories[i]);
	}
}



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
			ThreadData data(m_watchers.size(), m_directories[i], m_ct, m_callback, m_watchSubDiretories);
			watcher workerDataPair(make_pair(thread(watchDirectory, data), m_directories[i]));
			m_watchers.push_back(move(workerDataPair));
		}

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
		for (int i = 0; i < m_watchers.size(); i++)
		{
			m_watchers[i].first.join();
		}
		m_isWatching = false;
	}
}   

/**************************************************************************************************/
void DirectoryWatcher::AddDir(string directory)
{
	if (m_isWatching)
	{
		ThreadData data(m_watchers.size(), directory, m_ct, m_callback, m_watchSubDiretories);
		watcher workerDataPair(make_pair(thread(watchDirectory, data), directory));
		m_watchers.push_back(move(workerDataPair));
	}
	m_directories.push_back(directory);
}

/**************************************************************************************************/
void DirectoryWatcher::RemoveDir(string directory)
{
	auto result = std::find_if(m_watchers.begin(), m_watchers.end(),
		[directory](const watcher& element) {
		return element.second == directory;
	});
		
	if (result != std::end(m_watchers)) 
	{
		int workerId = std::distance(m_watchers.begin(), result);

		while (!m_ct.Cancel(workerId));
	}
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
ThreadData::ThreadData(int threadId, string directory, CancelationToken& token, dw_callback callback, bool watchSubtree)
	: ThreadId(threadId), Directory(directory), Token(token), Callback(callback), WatchSubtree(watchSubtree) { }
