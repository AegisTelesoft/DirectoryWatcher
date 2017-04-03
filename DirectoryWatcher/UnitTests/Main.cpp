#include <gtest/gtest.h>

#include "Fixtures.h"

static void callback(string directory, CallbackType type, string error)
{

}

TEST(DirectoryWatcher_create, DW_CreateAndWatch_Empty)
{
	DirectoryWatcher watcher(callback);
	watcher.Watch(false);
	watcher.Stop();
}

TEST(DirectoryWatcher_start, DW_CreateAndWatch_OneDir)
{
	CHAR pathToTempDir[MAX_PATH];
	GetTempPath(MAX_PATH, pathToTempDir);
	
	DirectoryWatcher watcher(string(pathToTempDir), callback);
	watcher.Watch(true);
	watcher.Stop();
}

TEST_F(FolderOperation, DW_CreateAndWatchSubdirs_DelteOneDirRecursive)
{
	CHAR pathToTempDir[MAX_PATH];
	GetTempPath(MAX_PATH, pathToTempDir);

	vector<string> dirs;

	char chars[] = "ABC";
	for (int i = 0; i < 3; i++)
	{
		dirs.push_back(string(pathToTempDir) + chars[i] + "\\");
	}

	DirectoryWatcher watcher(dirs, FolderOperation::DirectoryWatcherCallback);
	watcher.Watch(true);

	std::this_thread::sleep_for(std::chrono::milliseconds(1000));

	DeleteDirectoryAndAllSubfolders(string(pathToTempDir) + "A\\B\\B\\");

	std::this_thread::sleep_for(std::chrono::milliseconds(1000));


	EXPECT_EQ(CallbackCount(), 4);
	EXPECT_FALSE(WatcherFailed());
}

TEST_F(FolderOperation, DW_CreateAndWatch_DelteOneDirRecursive)
{
	CHAR pathToTempDir[MAX_PATH];
	GetTempPath(MAX_PATH, pathToTempDir);

	vector<string> dirs;

	char chars[] = "ABC";
	for (int i = 0; i < 3; i++)
	{
		dirs.push_back(string(pathToTempDir) + chars[i] + "\\");
	}

	DirectoryWatcher watcher(dirs, FolderOperation::DirectoryWatcherCallback);
	watcher.Watch(false);

	std::this_thread::sleep_for(std::chrono::milliseconds(1000));

	DeleteDirectoryAndAllSubfolders(string(pathToTempDir) + "A\\B\\B\\");

	std::this_thread::sleep_for(std::chrono::milliseconds(1000));


	EXPECT_EQ(CallbackCount(), 0);
	EXPECT_FALSE(WatcherFailed());
}

int main(int argc, char* argv[])
{
	testing::InitGoogleTest(&argc, argv);
	int result = RUN_ALL_TESTS();
	system("pause");
	return result;
}
