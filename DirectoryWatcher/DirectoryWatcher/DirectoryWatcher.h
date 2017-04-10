#pragma once

#include <string>
#include <vector>
#include <thread>

#include "CancelationToken.h"

enum CallbackType {
	Added,
	Deleted,
	Modified,
	RenamedFrom,
	RenamedTo,
	FailedToWatch
};

typedef std::function<void(std::string& directory, CallbackType type, std::string& details)> dw_callback;
typedef std::pair<std::thread, std::string> watcher;

class DirectoryWatcher
{
public:
	DirectoryWatcher(dw_callback callback);
	DirectoryWatcher(std::string directory, dw_callback callback);
	DirectoryWatcher(std::vector<std::string> directories, dw_callback callback);
	~DirectoryWatcher();

	bool Watch(bool watchSubDir);
	void Stop();
	void AddDir(std::string directory);
	void RemoveDir(std::string directory);

private:
	std::vector<std::string> m_directories;
	std::vector<watcher> m_watchers;
	CancelationToken m_ct;
	bool m_isWatching;
	bool m_watchSubDiretories;
	dw_callback m_callback;
};



