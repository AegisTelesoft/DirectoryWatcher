#pragma once

#include "gtest/gtest.h"
#include <Windows.h>
#include <string>
#include "Shlwapi.h"
#include "Shlobj.h"

using std::string;

class FolderOperation : public ::testing::Test
{
protected:
	FolderOperation();
	~FolderOperation();

	virtual void SetUp();
	virtual void TearDown();

private:
	CHAR m_pathToTemp[MAX_PATH];

private:
	void CreateFolder();
	void DeleteFolder();
};