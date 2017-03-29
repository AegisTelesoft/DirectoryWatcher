#include <iostream>

#include "DirectoryWatcher.h"

void callback(string directory, ChangeType type);

int main(int argc, char *argv[]) {

	vector<string> directories;
	directories.push_back(string("C:\\Users\\Egidijus Lileika\\Desktop\\test\\"));
	directories.push_back(string("C:\\Users\\Egidijus Lileika\\Desktop\\test - Copy\\"));
	directories.push_back(string("C:\\Users\\Egidijus Lileika\\Desktop\\test - Copy - Copy\\New folder (3)\\"));

	DirectoryWatcher watcher(directories, callback);

	watcher.Watch(false);
	system("pause");
	watcher.RemoveDir(string("C:\\Users\\Egidijus Lileika\\Desktop\\test\\"));
	system("pause");
	watcher.Stop();
	system("pause");
	return 0;
}

void callback(string directory, ChangeType type)
{
	cout << directory << " " << type << endl;
}