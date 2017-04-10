#include <iostream>
#include <string>
#include <vector>

using std::string;
using std::vector;
using std::cout;
using std::endl;


#include "DirectoryWatcher.h"

void callback(string directory, CallbackType type, string details)
{
	cout << directory << " " << details << endl;
}

int main(int argc, char *argv[]) {

	vector<string> directories;
	directories.push_back(string("C:\\Users\\Egidijus Lileika\\Desktop\\test\\"));

	DirectoryWatcher watcher(directories, callback);
	watcher.Watch(true);
	system("pause");
	watcher.AddDir(string("C:\\Users\\Egidijus Lileika\\Desktop\\A\\"));
	system("pause");
	watcher.RemoveDir(string("C:\\Users\\Egidijus Lileika\\Desktop\\test\\"));
	system("pause");
	watcher.Stop();
	system("pause");
	return 0;
}