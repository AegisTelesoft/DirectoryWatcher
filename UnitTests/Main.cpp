#include <gtest/gtest.h>
#include <vector>
#include <mutex>
#include <string>
#include <chrono>

#include "FileSystemOperation.h"
#include "DirectoryWatcher.h"

using std::vector;
using std::string;

typedef std::chrono::high_resolution_clock Clock;

/**************************************************************************************************/
/*                            Implementation specific helper class                                */
/**************************************************************************************************/
class Timeout
{
public:
	Timeout() {};

public:
	static bool WaitForCondition(int timeoutMS, std::function<bool()> condition)
	{
		bool timeout = false;
		auto startTime = Clock::now();

		while (!timeout && !condition())
		{
			if (std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now() - startTime).count() >= timeoutMS)
			{ 
				timeout = true;
				break;
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(5));
		}

		if (!timeout)
			return false;
		else
			return true;
	}
};

/**************************************************************************************************/
/*                          Test performed on FileSystemOperation fixture                         */
/**************************************************************************************************/
TEST_F(FileSystemOperation, DW_CreateAndWatchSubdirs_DelteOneDirRecursive)
{
	std::mutex mutex;

	int callbackCount = 0;
	int expectedCallbackCount = 2;
	bool failed = false;

#if defined (WINDOWS)
	string dir("A\\");
#elif defined (LINUX)
	string dir("A/");
#endif

	DirectoryWatcher watcher(string(GetTestDirPath() + dir),
		[&mutex, &failed, &callbackCount] (string directory, CallbackType type, string details)
	{
			std::unique_lock<std::mutex> lock(mutex);

			callbackCount++;

			if (type == FailedToWatch)
				failed = true;

			lock.unlock();
	});

	watcher.Watch(true);

#if defined (WINDOWS)
	string dirToDelete("A\\B\\C\\");
#elif defined (LINUX)
	string dirToDelete("A/B/C/");
#endif

	while(watcher.GetStatus() != Watching){
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}

	DeleteDirectoryAndAllSubfolders(GetTestDirPath() + dirToDelete);


	Timeout::WaitForCondition(100, [&mutex, &callbackCount, &expectedCallbackCount]() 
	{
		std::lock_guard<std::mutex> lock(mutex);
		return (callbackCount == expectedCallbackCount);
	});

	EXPECT_EQ(callbackCount, expectedCallbackCount);
	EXPECT_FALSE(failed);
}

/**************************************************************************************************/
TEST_F(FileSystemOperation, DW_CreateAndWatch_DelteOneDirRecursive)
{
	std::mutex mutex;

	int callbackCount = 0;
	bool failed = false;

#if defined (WINDOWS)
	string dir("A\\");
#elif defined (LINUX)
	string dir("A/");
#endif

	DirectoryWatcher watcher(string(GetTestDirPath() + dir),
		[&mutex, &failed, &callbackCount](string directory, CallbackType type, string details)
	{
		std::unique_lock<std::mutex> lock(mutex);

		callbackCount++;

		if (type == FailedToWatch)
			failed = true;

		lock.unlock();
	});

	watcher.Watch(false);

#if defined (WINDOWS)
	string dirToDelete("A\\B\\C\\");
#elif defined (LINUX)
	string dirToDelete("A/B/C/");
#endif

	DeleteDirectoryAndAllSubfolders(GetTestDirPath() + dirToDelete);

	Timeout::WaitForCondition(100, [&mutex, &callbackCount]() 
	{
		std::lock_guard<std::mutex> lock(mutex);
		return (callbackCount != 0);
	});

	EXPECT_EQ(callbackCount, 0);
	EXPECT_FALSE(failed);
}

/**************************************************************************************************/
TEST_F(FileSystemOperation, DW_AddDirectoryWatchSubDirs_AndAndDelete)
{
	std::mutex mutex;

	int callbackCount = 0;
	int expectedCallbackCount = 4;
	bool failed = false;

	DirectoryWatcher watcher([&mutex, &failed, &callbackCount](string directory, CallbackType type, string details)
	{
		std::unique_lock<std::mutex> lock(mutex);

		callbackCount++;

		if (type == FailedToWatch)
			failed = true;

		lock.unlock();
	});

	watcher.Watch(true);

#if defined (WINDOWS)
	string dir("A\\");
#elif defined (LINUX)
	string dir("A/");
#endif

	watcher.AddDir(GetTestDirPath() + dir);

#if defined (WINDOWS)
	string dirToDelete("A\\B\\");
#elif defined (LINUX)
	string dirToDelete("A/B/");
#endif

	while(watcher.GetStatus() != Watching){
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}


	DeleteDirectoryAndAllSubfolders(GetTestDirPath() + dirToDelete);

	Timeout::WaitForCondition(100, [&mutex, &callbackCount, &expectedCallbackCount]()
	{
		std::lock_guard<std::mutex> lock(mutex);
		return (callbackCount == expectedCallbackCount);
	});


	EXPECT_EQ(callbackCount, expectedCallbackCount);
	EXPECT_FALSE(failed);
}

/**************************************************************************************************/
TEST_F(FileSystemOperation, DW_AddDirectory_AndAndDelete)
{
	std::mutex mutex;

	int callbackCount = 0;
	bool failed = false;

	DirectoryWatcher watcher([&mutex, &failed, &callbackCount](string directory, CallbackType type, string details)
	{
		std::unique_lock<std::mutex> lock(mutex);

		callbackCount++;

		if (type == FailedToWatch)
			failed = true;

		lock.unlock();
	});

	watcher.Watch(false);

#if defined (WINDOWS)
	string dir("A\\");
#elif defined (LINUX)
	string dir("A/");
#endif

	watcher.AddDir(GetTestDirPath() + dir);

#if defined (WINDOWS)
	string dirToDelete("A\\B\\C\\");
#elif defined (LINUX)
	string dirToDelete("A/B/C/");
#endif

	DeleteDirectoryAndAllSubfolders(GetTestDirPath() + dirToDelete);

	Timeout::WaitForCondition(100, [&mutex, &callbackCount]()
	{
		std::lock_guard<std::mutex> lock(mutex);
		return (callbackCount != 0);
	});
	EXPECT_EQ(callbackCount, 0);
	EXPECT_FALSE(failed);
}

/**************************************************************************************************/
TEST_F(FileSystemOperation, DW_RemoveDirSubDirs_RemoveAndDelete)
{
	std::mutex mutex;

	int callbackCount = 0;
	bool failed = false;

#if defined (WINDOWS)
	string dir("A\\");
#elif defined (LINUX)
	string dir("A/");
#endif

	DirectoryWatcher watcher(GetTestDirPath() + dir,
		[&mutex, &failed, &callbackCount](string directory, CallbackType type, string details)
	{
		std::unique_lock<std::mutex> lock(mutex);

		callbackCount++;

		if (type == FailedToWatch)
			failed = true;

		lock.unlock();
	});

	watcher.Watch(true);

	watcher.RemoveDir(GetTestDirPath() + dir);

#if defined (WINDOWS)
	string dirToDelete("A\\B\\C\\");
#elif defined (LINUX)
	string dirToDelete("A/B/C/");
#endif

	DeleteDirectoryAndAllSubfolders(GetTestDirPath() + dirToDelete);

	Timeout::WaitForCondition(100, [&mutex, &callbackCount]()
	{
		std::lock_guard<std::mutex> lock(mutex);
		return (callbackCount != 0);
	});
	EXPECT_EQ(callbackCount, 0);
	EXPECT_FALSE(failed);
}

/**************************************************************************************************/
TEST_F(FileSystemOperation, DW_RemoveDir_RemoveAndDelete)
{
	std::mutex mutex;

	int callbackCount = 0;
	bool failed = false;

#if defined (WINDOWS)
	string dir("A\\");
#elif defined (LINUX)
	string dir("A/");
#endif


	DirectoryWatcher watcher(GetTestDirPath() + dir,
		[&mutex, &failed, &callbackCount](string directory, CallbackType type, string details)
	{
		std::unique_lock<std::mutex> lock(mutex);

		callbackCount++;

		if (type == FailedToWatch)
			failed = true;

		lock.unlock();
	});

 	watcher.Watch(false);

	watcher.RemoveDir(GetTestDirPath() + dir);

#if defined (WINDOWS)
	string dirToDelete("A\\B\\C\\");
#elif defined (LINUX)
	string dirToDelete("A/B/C/");
#endif

	DeleteDirectoryAndAllSubfolders(GetTestDirPath() + dirToDelete);

	Timeout::WaitForCondition(100, [&mutex, &callbackCount]()
	{
		std::lock_guard<std::mutex> lock(mutex);
		return (callbackCount != 0);
	});
	EXPECT_EQ(callbackCount, 0);
	EXPECT_FALSE(failed);
}

/**************************************************************************************************/
/*                                                                                                */
/**************************************************************************************************/
int main(int argc, char* argv[])
{
	testing::InitGoogleTest(&argc, argv);
	int result = RUN_ALL_TESTS();
#if defined (WINDOWS)
	system("pause");
#endif
	return result;
}
