#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <chrono>
#include <mutex>
#include <algorithm>

#include <windows.h>
#include <tchar.h>
#include <comdef.h>

#include "CancelationToken.h"

using std::vector;
using std::thread;
using std::string;
using std::mutex;
using std::cout;
using std::endl;

enum ChangeType {
	Added,
	Deleted,
	Modified,
	RenamedFrom,
	RenamedTo
};

typedef std::function<void(string&, ChangeType)> dw_callback;

class DirectoryWatcher
{
public:
	DirectoryWatcher(dw_callback callback);
	DirectoryWatcher(string& directory, dw_callback callback);
	DirectoryWatcher(vector<string>& directories, dw_callback callback);
	~DirectoryWatcher();

	bool Watch(bool watchSubDir);
	void Stop();
	void AddDir(string& directory);
	void RemoveDir(string& directory);

private:
	vector<string> m_directories;
	vector<string> m_newDirectories;
	vector<string> m_dirsToRemove;
	thread m_masterThread;
	mutex m_mutex;
	CancelationToken m_ct;
	bool m_isWatching;
	dw_callback m_callback;
};


