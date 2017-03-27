#include <iostream>

#include "DirectoryWatcher.h"


int main(int argc, char *argv[]) {

	vector<string> directories;

	for (int i = 0; i < 4; i++) {
		directories.push_back("asdasd");
	}

	DirectoryWatcher watcher(directories);

	watcher.Watch(false);

	system("pause");
	watcher.Stop();

	return 0;
}