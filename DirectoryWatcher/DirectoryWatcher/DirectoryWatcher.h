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
	void AddDirectory(const char* directory);
	void RemoveDirectory(int id);
	void RemoveDirectory(const char* directory);

private:
	vector<struct Directory> m_directories;
	thread m_masterThread;
	mutex m_mutex;
	bool m_isWatching = false;
	CancelationToken m_ct;
	CancelationToken* m_ctPtr = &m_ct;
};

struct Directory 
{
	Directory(string path, int id) 
		: Path(path), Id(id) { };

	string Path;
	int Id;
};

struct WorkerThreadData 
{
	WorkerThreadData(struct Directory dir, int threadId, CancelationToken* token)
		: directory(dir), threadId(threadId), token(token) {}

	struct Directory directory;
	int threadId;
	CancelationToken* token;
};

struct MasterThreadData {
	MasterThreadData(vector<struct Directory> &directories, CancelationToken* token) 
		: directories(directories), token(token) {}

	vector<struct Directory> directories;
	CancelationToken* token;
};

void masterThreadTask(struct MasterThreadData data);
void workerThreadTask(struct WorkerThreadData data);