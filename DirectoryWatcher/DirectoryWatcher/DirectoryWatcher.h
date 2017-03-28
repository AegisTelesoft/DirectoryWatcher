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

enum ChangeType;

void masterThreadTask(struct MasterThreadData data);
void workerThreadTask(struct WorkerThreadData data);
int watchDirectory(WorkerThreadData data);

typedef std::function<void(string&, ChangeType)> dw_callback;

class DirectoryWatcher
{
public:
	DirectoryWatcher(dw_callback callback);
	DirectoryWatcher(string& directory, dw_callback callback);
	DirectoryWatcher(vector<string>& directories, dw_callback callback);
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
	std::function<void(string&, ChangeType)> m_callback;
};

struct WorkerThreadData
{
	WorkerThreadData(string dir, int threadId, CancelationToken* token, dw_callback callback);

	string directory;
	CancelationToken* token;
	int threadId;
	dw_callback callback;
};

struct MasterThreadData 
{
	MasterThreadData(vector<string>* directories, vector<string>* dirsToRemove, CancelationToken* token, dw_callback callback);

	vector<string>* dirsToRemove;
	vector<string>* newDirectories;
	CancelationToken* token;
	dw_callback callback;
};

enum ChangeType { 
	FileNameChanged, 
	FileCreated, 
	FileDeleted, 
	DirectoryNameChanged, 
	DirectoryCreated, 
	DirectoryDeleted
};
