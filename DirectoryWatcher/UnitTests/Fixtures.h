#pragma once

#include "gtest/gtest.h"
#include <Windows.h>
#include <string>
#include <mutex>
#include "Shlwapi.h"
#include "Shlobj.h"
#include "Strsafe.h"

#include "DirectoryWatcher.h"

using std::string;

class FolderOperation : public ::testing::Test
{
protected:
	FolderOperation();
	~FolderOperation();
	virtual void SetUp();
	static LONG DeleteDirectoryAndAllSubfolders(string wzDirectory);
	static void DirectoryWatcherCallback(string directory, CallbackType type, string error);
	unsigned CallbackCount();
	bool WatcherFailed();

protected:
	static unsigned m_CallbackCount;
	static std::mutex m_CbMutex;
	static bool m_FailedToWatch;

private:
	CHAR m_pathToTemp[MAX_PATH];

private:
	void CreateFolder();
	void DeleteFolder();
};