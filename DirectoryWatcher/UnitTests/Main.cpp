#include <gtest/gtest.h>

#include "FileSystemOperation.h"

/**************************************************************************************************/
static void callback(string directory, CallbackType type, string error)
{

}

/**************************************************************************************************/
/*                                           Simple tests						                  */
/**************************************************************************************************/
TEST(DirectoryWatcher_create, DW_CreateAndWatch_Empty)
{
	DirectoryWatcher watcher(callback);
	watcher.Watch(false);
	watcher.Stop();
}

/**************************************************************************************************/
TEST(DirectoryWatcher_start, DW_CreateAndWatch_OneDir)
{
	CHAR pathToTempDir[MAX_PATH];
	GetTempPath(MAX_PATH, pathToTempDir);
	
	DirectoryWatcher watcher(string(pathToTempDir), callback);
	watcher.Watch(true);
	watcher.Stop();
}

/**************************************************************************************************/
/*                          Test performed on FileSystemOperation fixture                         */
/**************************************************************************************************/
TEST_F(FileSystemOperation, DW_CreateAndWatchSubdirs_DelteOneDirRecursive)
{
	CHAR pathToTempDir[MAX_PATH];
	GetTempPath(MAX_PATH, pathToTempDir);

	vector<string> dirs;

	char chars[] = "ABC";
	for (int i = 0; i < 3; i++)
	{
		dirs.push_back(string(pathToTempDir) + chars[i] + "\\");
	}

	DirectoryWatcher watcher(dirs, FileSystemOperation::DirectoryWatcherCallback);
	watcher.Watch(true);

	DeleteDirectoryAndAllSubfolders(string(pathToTempDir) + "A\\B\\B\\");

	EXPECT_EQ(CallbackCount(), 8);
	EXPECT_FALSE(WatcherFailed());
}

/**************************************************************************************************/
TEST_F(FileSystemOperation, DW_CreateAndWatch_DelteOneDirRecursive)
{
	CHAR pathToTempDir[MAX_PATH];
	GetTempPath(MAX_PATH, pathToTempDir);

	vector<string> dirs;

	char chars[] = "ABC";
	for (int i = 0; i < 3; i++)
	{
		dirs.push_back(string(pathToTempDir) + chars[i] + "\\");
	}

	DirectoryWatcher watcher(dirs, FileSystemOperation::DirectoryWatcherCallback);
	watcher.Watch(false);

	DeleteDirectoryAndAllSubfolders(string(pathToTempDir) + "A\\B\\B\\");

	EXPECT_EQ(CallbackCount(), 0);
	EXPECT_FALSE(WatcherFailed());
}

/**************************************************************************************************/
TEST_F(FileSystemOperation, DW_AddDirectoryWatchSubDirs_AndAndDelete)
{
	CHAR pathToTempDir[MAX_PATH];
	GetTempPath(MAX_PATH, pathToTempDir);

	DirectoryWatcher watcher(string(pathToTempDir) + "B\\C\\", 
		FileSystemOperation::DirectoryWatcherCallback);

	watcher.Watch(true);

	watcher.AddDir(string(pathToTempDir) +"A\\B\\C");

	DeleteDirectoryAndAllSubfolders(string(pathToTempDir) + "A\\B\\C\\A\\");

	EXPECT_EQ(CallbackCount(), 2);
	EXPECT_FALSE(WatcherFailed());
}

/**************************************************************************************************/
TEST_F(FileSystemOperation, DW_AddDirectory_AndAndDelete)
{
	CHAR pathToTempDir[MAX_PATH];
	GetTempPath(MAX_PATH, pathToTempDir);

	DirectoryWatcher watcher(string(pathToTempDir) + "B\\C\\",
		FileSystemOperation::DirectoryWatcherCallback);

	watcher.Watch(false);

	watcher.AddDir(string(pathToTempDir) + "A\\B\\C");

	DeleteDirectoryAndAllSubfolders(string(pathToTempDir) + "A\\B\\C\\A\\");

	EXPECT_EQ(CallbackCount(), 0);
	EXPECT_FALSE(WatcherFailed());
}

/**************************************************************************************************/
TEST_F(FileSystemOperation, DW_RemoveDirSubDirs_RemoveAndDelete)
{
	CHAR pathToTempDir[MAX_PATH];
	GetTempPath(MAX_PATH, pathToTempDir);

	vector<string> dirs;

	char chars[] = "ABC";
	for (int i = 0; i < 3; i++)
	{
		dirs.push_back(string(pathToTempDir) + chars[i] + "\\" + chars[i] + "\\");
	}

	DirectoryWatcher watcher(dirs, FileSystemOperation::DirectoryWatcherCallback);

	watcher.Watch(true);

	watcher.RemoveDir(string(pathToTempDir) + "B\\B\\");

	DeleteDirectoryAndAllSubfolders(string(pathToTempDir) + "B\\B\\C\\A\\");

	EXPECT_EQ(CallbackCount(), 0);
	EXPECT_FALSE(WatcherFailed());
}

/**************************************************************************************************/
TEST_F(FileSystemOperation, DW_RemoveDir_RemoveAndDelete)
{
	CHAR pathToTempDir[MAX_PATH];
	GetTempPath(MAX_PATH, pathToTempDir);

	vector<string> dirs;

	char chars[] = "ABC";
	for (int i = 0; i < 3; i++)
	{
		dirs.push_back(string(pathToTempDir) + chars[i] + "\\");
	}

	DirectoryWatcher watcher(dirs, FileSystemOperation::DirectoryWatcherCallback);

 	watcher.Watch(false);

	watcher.RemoveDir(string(pathToTempDir) + "B\\");

	DeleteDirectoryAndAllSubfolders(string(pathToTempDir) + "B\\");

	EXPECT_EQ(CallbackCount(), 0);
	EXPECT_FALSE(WatcherFailed());
}

/**************************************************************************************************/
/*                                                                                                */
/**************************************************************************************************/
int main(int argc, char* argv[])
{
	testing::InitGoogleTest(&argc, argv);
	int result = RUN_ALL_TESTS();
	system("pause");
	return result;
}
