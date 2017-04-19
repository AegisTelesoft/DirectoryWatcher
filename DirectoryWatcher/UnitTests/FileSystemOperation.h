#pragma once

#include "gtest/gtest.h"
#include <Windows.h>

class FileSystemOperation : public ::testing::Test
{
protected:
	FileSystemOperation();
	~FileSystemOperation();
	virtual void SetUp();
	static LONG DeleteDirectoryAndAllSubfolders(std::string wzDirectory);
	std::string GetTestDirPath();

private:
	std::string m_pathToTemp;
};