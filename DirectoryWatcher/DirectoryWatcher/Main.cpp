#include <iostream>

#include "DirectoryWatcher.h"


int main(int argc, char *argv[]) {

	vector<string> directories;

	for (int i = 1; i <= 4; i++) {
		directories.push_back(string("thread " + i));
	}

	DirectoryWatcher watcher(directories);

	watcher.Watch(false);

	system("pause");
	watcher.AddDirectory(string("thread 5"));
	watcher.AddDirectory(string("thread 6"));
	system("pause");
	watcher.RemoveDirectory(string("thread 5"));
	watcher.RemoveDirectory(string("thread 6"));
	system("pause");
	watcher.Stop();
	system("pause");
	return 0;
}