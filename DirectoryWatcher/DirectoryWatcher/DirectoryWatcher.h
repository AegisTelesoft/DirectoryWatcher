#pragma once

#include <iostream>
#include <string>
#include <thread>
#include <vector>

using std::vector;
using std::thread;
using std::string;

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
	vector<Directory> m_directories;
	thread m_masterThread;
};

struct Directory {
	Directory(string path, unsigned id) : Path(path), Id(id) {
	
	};
	string Path;
	unsigned Id;
};