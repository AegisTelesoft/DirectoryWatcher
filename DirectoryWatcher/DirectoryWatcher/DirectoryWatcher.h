#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <algorithm>

#include <windows.h>
#include <tchar.h>
#include <comdef.h>

#include <cstdlib>

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
VOID CALLBACK ReadDirChangesCallback(DWORD errorCode, DWORD bytesTransfered, LPOVERLAPPED lpOverlapped);

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
	void AddDir(string& directory);
	void RemoveDir(string& directory);

private:
	vector<string> m_newDirectories;
	vector<string> m_dirsToRemove;
	thread m_masterThread;
	mutex m_mutex;
	CancelationToken m_ct;
	bool m_isWatching;
	std::function<void(string&, ChangeType)> m_callback;
};

struct WorkerThreadData
{
	WorkerThreadData(string dir, bool watchSubtree, int threadId, CancelationToken* token, dw_callback callback);

	string directory;
	CancelationToken* token;
	int threadId;
	dw_callback callback;
	bool watchSubtree;
};

struct MasterThreadData 
{
	MasterThreadData(vector<string>* directories, bool watchSubtree, vector<string>* dirsToRemove, CancelationToken* token, dw_callback callback);

	vector<string>* dirsToRemove;
	vector<string>* newDirectories;
	CancelationToken* token;
	dw_callback callback;
	bool watchSubtree;
};

enum ChangeType { 
	Added, 
	Deleted, 
	Modified, 
	Renamed
};
