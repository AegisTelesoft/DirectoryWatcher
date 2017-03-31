#include "Fixtures.h"

/**************************************************************************************************/
bool CreateDirectoriesRecursive(int iteration, int maxIterations, CHAR rootDir[], string lastPath)
{
	char chars[] = "ABCDEF";

	for (int i = 0; i <= 5; i++)
	{
		string tmpPath = "\\" + lastPath + chars[i];

		LPCTSTR dirPath = PathCombine("", rootDir, tmpPath.c_str());

		if (SHCreateDirectoryEx(NULL, dirPath, NULL) == ERROR_SUCCESS)
		{

			if (iteration <= maxIterations)
			{
				// Continue recursion
				CreateDirectoriesRecursive(iteration + 1, maxIterations, rootDir, tmpPath);
			}
			else
			{
				// Successfully created directories recursively
				return true;
			}
		}
		else 
		{
			return false;
		}
	}
}

/**************************************************************************************************/
/*                                                                                                */
/**************************************************************************************************/
FolderOperation::FolderOperation() 
{
	m_pathToTemp[MAX_PATH];
	GetTempPath(MAX_PATH, m_pathToTemp);
};

/**************************************************************************************************/
FolderOperation::~FolderOperation() {};

/**************************************************************************************************/
void FolderOperation::TearDown()
{
	DeleteFolder();
}

/**************************************************************************************************/
void FolderOperation::SetUp()
{
	CreateFolder();
}

/**************************************************************************************************/
void FolderOperation::CreateFolder()
{
	if (!CreateDirectoriesRecursive(1, 4, m_pathToTemp, ""))
	{
		throw std::exception("\nERROR: Could not create directories correctly!\n");
	}
}

/**************************************************************************************************/
void FolderOperation::DeleteFolder()
{
	char chars[] = "ABCDEF";

	for (int i = 0; i <= 5; i++)
	{
		string tmpPath = "\\" + chars[i];
		LPCTSTR dirPath = PathCombine("", m_pathToTemp, tmpPath.c_str());
		RemoveDirectory(dirPath);
	}
}