#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <algorithm>
#include <chrono>

#ifdef _WIN32
#include <windows.h>
#include <tchar.h>
#include "atlstr.h"
#endif

#include "CancelationToken.h"

using std::vector;
using std::thread;
using std::string;
using std::mutex;
using std::cout;
using std::endl;

void masterThreadTask(struct MasterThreadData data);
void workerThreadTask(struct WorkerThreadData data);
int watchDirectory(WorkerThreadData data);

class DirectoryWatcher
{
public:
	DirectoryWatcher();
	DirectoryWatcher(string& directory);
	DirectoryWatcher(vector<string>& directories);
	~DirectoryWatcher();

	void Watch(bool watchSubDir);
	void Stop();
	void AddDirectory(string& directory);
	void RemoveDirectory(string& directory);

private:
	vector<string> m_newDirectories;
	vector<string> m_dirsToRemove;
	thread m_masterThread;
	mutex m_mutex;
	CancelationToken m_ct;
	bool m_isWatching = false;
};

struct WorkerThreadData
{
	WorkerThreadData(string dir, int threadId, CancelationToken* token);

	string directory;
	CancelationToken* token;
	int threadId;
};

struct MasterThreadData 
{
	MasterThreadData(vector<string>* directories, vector<string>* dirsToRemove, CancelationToken* token);

	vector<string>* dirsToRemove;
	vector<string>* newDirectories;
	CancelationToken* token;
};
