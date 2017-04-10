#include <gtest/gtest.h>
#include <vector>
#include <string>

#include "FileSystemOperation.h"
#include "DirectoryWatcher.h"

using std::vector;
using std::string;


/**************************************************************************************************/
/*                            Implementation specific helper class                                */
/**************************************************************************************************/
class TestResult
{
public:
	TestResult(int timeout) : TimeOutCurrent(timeout), TimeOut(timeout){};
public:
	int CallbackCount = 0;
	int TimeOut;
	int TimeOutCurrent;
	bool Failure = false;
};

/**************************************************************************************************/
/*                          Test performed on FileSystemOperation fixture                         */
/**************************************************************************************************/
TEST_F(FileSystemOperation, DW_CreateAndWatchSubdirs_DelteOneDirRecursive)
{
	TestResult result(75);
	std::mutex mutex;

	DirectoryWatcher watcher(string(GetTestDirPath() + "A\\"),
		[&mutex, &result](string directory, CallbackType type, string details) 
	{
			std::unique_lock<std::mutex> lock(mutex);

			result.CallbackCount++;
			result.TimeOutCurrent = result.TimeOut;

			if (type == FailedToWatch)
				result.Failure = true;

			lock.unlock();
	});

	watcher.Watch(true);

	DeleteDirectoryAndAllSubfolders(GetTestDirPath() + "A\\B\\C\\");

	// Wait for results with minimal main thread interuption. If wait time-outs, loop breaks.
	while (true)
	{
		std::unique_lock<std::mutex> lock(mutex);
		if (result.TimeOutCurrent <= 0)
			break;

		result.TimeOutCurrent--;
		lock.unlock();

		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}

	EXPECT_EQ(result.CallbackCount, 2);
	EXPECT_FALSE(result.Failure);
}

/**************************************************************************************************/
TEST_F(FileSystemOperation, DW_CreateAndWatch_DelteOneDirRecursive)
{
	TestResult result(75);
	std::mutex mutex;

	DirectoryWatcher watcher(string(GetTestDirPath() + "A\\"),
		[&mutex, &result](string directory, CallbackType type, string details) 
	{
		std::unique_lock<std::mutex> lock(mutex);

		result.CallbackCount++;
		result.TimeOutCurrent = result.TimeOut;

		if (type == FailedToWatch)
			result.Failure = true;

		lock.unlock();
	});

	watcher.Watch(false);

	DeleteDirectoryAndAllSubfolders(GetTestDirPath() + "A\\B\\C\\");

	while (true)
	{
		std::unique_lock<std::mutex> lock(mutex);
		if (result.TimeOutCurrent <= 0)
			break;

		result.TimeOutCurrent--;
		lock.unlock();
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}

	EXPECT_EQ(result.CallbackCount, 0);
	EXPECT_FALSE(result.Failure);
}

/**************************************************************************************************/
TEST_F(FileSystemOperation, DW_AddDirectoryWatchSubDirs_AndAndDelete)
{
	TestResult result(75);
	std::mutex mutex;

	DirectoryWatcher watcher([&mutex, &result](string directory, CallbackType type, string details) 
	{
		std::unique_lock<std::mutex> lock(mutex);

		result.CallbackCount++;
		result.TimeOutCurrent = result.TimeOut;

		if (type == FailedToWatch)
			result.Failure = true;

		lock.unlock();
	});

	watcher.Watch(true);

	watcher.AddDir(GetTestDirPath() + "A\\");

	DeleteDirectoryAndAllSubfolders(GetTestDirPath() + "A\\B\\");

	while (true)
	{
		std::unique_lock<std::mutex> lock(mutex);
		if (result.TimeOutCurrent <= 0)
			break;

		result.TimeOutCurrent--;
		lock.unlock();
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}

	EXPECT_EQ(result.CallbackCount, 4);
	EXPECT_FALSE(result.Failure);
}

/**************************************************************************************************/
TEST_F(FileSystemOperation, DW_AddDirectory_AndAndDelete)
{
	TestResult result(75);
	std::mutex mutex;

	DirectoryWatcher watcher([&mutex, &result](string directory, CallbackType type, string details) 
	{
		std::unique_lock<std::mutex> lock(mutex);

		result.CallbackCount++;
		result.TimeOutCurrent = result.TimeOut;

		if (type == FailedToWatch)
			result.Failure = true;

		lock.unlock();
	});

	watcher.Watch(false);

	watcher.AddDir(GetTestDirPath() + "A\\");

	DeleteDirectoryAndAllSubfolders(GetTestDirPath() + "A\\B\\C\\");

	while (true)
	{
		std::unique_lock<std::mutex> lock(mutex);
		if (result.TimeOutCurrent <= 0)
			break;

		result.TimeOutCurrent--;
		lock.unlock();
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}

	EXPECT_EQ(result.CallbackCount, 0);
	EXPECT_FALSE(result.Failure);
}

/**************************************************************************************************/
TEST_F(FileSystemOperation, DW_RemoveDirSubDirs_RemoveAndDelete)
{
	TestResult result(75);
	std::mutex mutex;

	DirectoryWatcher watcher(GetTestDirPath() + "A\\",
		[&mutex, &result](string directory, CallbackType type, string details) 
	{
		std::unique_lock<std::mutex> lock(mutex);

		result.CallbackCount++;
		result.TimeOutCurrent = result.TimeOut;

		if (type == FailedToWatch)
			result.Failure = true;

		lock.unlock();
	});

	watcher.Watch(true);

	watcher.RemoveDir(GetTestDirPath() + "A\\");

	DeleteDirectoryAndAllSubfolders(GetTestDirPath() + "A\\B\\C\\");

	while (true)
	{
		std::unique_lock<std::mutex> lock(mutex);
		if (result.TimeOutCurrent <= 0)
			break;

		result.TimeOutCurrent--;
		lock.unlock();
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}

	EXPECT_EQ(result.CallbackCount, 0);
	EXPECT_FALSE(result.Failure);
}

/**************************************************************************************************/
TEST_F(FileSystemOperation, DW_RemoveDir_RemoveAndDelete)
{
	TestResult result(75);
	std::mutex mutex;

	DirectoryWatcher watcher(GetTestDirPath() + "A\\",
		[&mutex, &result](string directory, CallbackType type, string details) 
	{
		std::unique_lock<std::mutex> lock(mutex);

		result.CallbackCount++;
		result.TimeOutCurrent = result.TimeOut;

		if (type == FailedToWatch)
			result.Failure = true;

		lock.unlock();
	});

 	watcher.Watch(false);

	watcher.RemoveDir(GetTestDirPath() + "A\\");

	DeleteDirectoryAndAllSubfolders(GetTestDirPath() + "A\\B\\C\\");

	while (true)
	{
		std::unique_lock<std::mutex> lock(mutex);
		if (result.TimeOutCurrent <= 0)
			break;

		result.TimeOutCurrent--;
		lock.unlock();
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}

	EXPECT_EQ(result.CallbackCount, 0);
	EXPECT_FALSE(result.Failure);
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
