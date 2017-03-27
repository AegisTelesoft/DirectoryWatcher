#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <mutex>

#include <chrono>

#include "CancelationToken.h"

using std::vector;
using std::thread;
using std::string;
using std::mutex;

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
	void RemoveDirectory(int id);
	void RemoveDirectory(string& directory);

private:
	vector<struct Directory> m_directories;
	thread m_masterThread;
	mutex m_mutex;
	CancelationToken m_ct;
	CancelationToken* m_ctPtr = &m_ct;
	bool m_isWatching = false;
};

struct Directory 
{
	Directory(string path, int id);

	string Path;
	int Id;
};

struct WorkerThreadData 
{
	WorkerThreadData(Directory dir, int threadId, CancelationToken* token);

	struct Directory directory;
	CancelationToken* token;
	int threadId;
};

struct MasterThreadData {
	MasterThreadData(vector<Directory> &directories, CancelationToken* token);

	vector<Directory> directories;
	CancelationToken* token;
};

void masterThreadTask(struct MasterThreadData data);
void workerThreadTask(struct WorkerThreadData data);