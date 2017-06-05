#pragma once

#include "gtest/gtest.h"

#if defined (_WIN32)
	#define WINDOWS
#elif defined (__linux__) || defined (__linux)
	#define LINUX
#endif


class FileSystemOperation : public ::testing::Test
{
protected:
	FileSystemOperation();
	~FileSystemOperation();
	virtual void SetUp();
	static void DeleteDirectoryAndAllSubfolders(std::string wzDirectory);
	std::string GetTestDirPath();

private:
	std::string m_pathToTemp;
};