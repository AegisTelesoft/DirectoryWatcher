#include <iostream>

#include "DirectoryWatcher.h"


int main(int argc, char *argv[]) {

	vector<string> directories;
	directories.push_back(string("C:\\Users\\Egidijus Lileika\\Desktop\\test\\"));

	DirectoryWatcher watcher(directories);

	watcher.Watch(false);
	system("pause");
	//watcher.RemoveDirectory(string("thread 5"));
	//watcher.RemoveDirectory(string("thread 6"));
	//system("pause");
	watcher.Stop();
	system("pause");
	return 0;
}