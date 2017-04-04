#include <iostream>

#include "DirectoryWatcher.h"

void callback(string directory, CallbackType type, string error);

int main(int argc, char *argv[]) {

	vector<string> directories;
	directories.push_back(string("C:\\Users\\Egidijus Lileika\\Desktop\\test\\"));

	DirectoryWatcher watcher(directories, callback);
	watcher.Watch(true);
	system("pause");
	watcher.RemoveDir(string("C:\\Users\\Egidijus Lileika\\Desktop\\test\\"));
	//system("pause");
	//watcher.RemoveDir(string("C:\\Users\\Egidijus Lileika\\Desktop\\test\\"));
	system("pause");
	watcher.Stop();
	system("pause");
	return 0;
}

void callback(string directory, CallbackType type, string error)
{
	cout << directory << " " << type << endl;
}