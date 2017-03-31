#include <gtest/gtest.h>

#include "DirectoryWatcher.h"
#include "Fixtures.h"

static void callback(string directory, ChangeType type)
{
	cout << directory << " " << type << endl;
}

TEST(DirectoryWatcher_create, DW_CreateAndWatch_Empty)
{
	DirectoryWatcher watcher(callback);
	watcher.Watch(false);
}

TEST(DirectoryWatcher_start, DW_CreateAndWatch_One_Dir)
{
	CHAR pathToTempDir[MAX_PATH];
	GetTempPath(MAX_PATH, pathToTempDir);
	
	DirectoryWatcher watcher(string(pathToTempDir), callback);
	watcher.Watch(true);
}

TEST(DirectoryWatcher_start, create_with_many_dirs)
{
	CHAR pathToTempDir[MAX_PATH];
	GetTempPath(MAX_PATH, pathToTempDir);

	vector<string> dirs;
	for (int i = 0; i < 5; i++)
	{
		dirs.push_back(string(pathToTempDir));
	}

	DirectoryWatcher watcher(dirs, callback);
	watcher.Watch(true);
}

TEST_F(FolderOperation, test)
{
	CHAR pathToTempDir[MAX_PATH];
	GetTempPath(MAX_PATH, pathToTempDir);

	DirectoryWatcher watcher(string(pathToTempDir), callback);
	watcher.Watch(true);
}

int main(int argc, char* argv[])
{
	testing::InitGoogleTest(&argc, argv);
	int result = RUN_ALL_TESTS();
	system("pause");
	return result;
}
