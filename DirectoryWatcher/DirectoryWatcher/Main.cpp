#include <iostream>

#include "DirectoryWatcher.h"

void callback(string directory, ChangeType type);

int main(int argc, char *argv[]) {

	vector<string> directories;
	directories.push_back(string("C:\\Users\\Egidijus Lileika\\Desktop\\test\\"));

	DirectoryWatcher watcher(directories, callback);

	watcher.Watch(false);
	system("pause");
	//watcher.RemoveDirectory(string("thread 5"));
	//watcher.RemoveDirectory(string("thread 6"));
	//system("pause");
	watcher.Stop();
	system("pause");
	return 0;
}

void callback(string directory, ChangeType type)
{
	cout << directory << " " << type << endl;
}