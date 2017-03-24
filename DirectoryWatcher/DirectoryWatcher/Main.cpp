#include <iostream>

#include "DirectoryWatcher.h"


int main(int argc, char *argv[]) {

	vector<string> directories;

	for (int i = 0; i < 4; i++) {
		directories.push_back("asdasd");
	}

	DirectoryWatcher watcher(directories);

	watcher.Watch(false);
	//uncomment this shit below to check how stop function handles thread termination
	/*system("pause");
	watcher.Stop();*/

	std::cout << "Main scope end." << std::endl;
	return 0;
}