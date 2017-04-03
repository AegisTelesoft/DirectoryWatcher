#include "Fixtures.h"

unsigned FolderOperation::m_CallbackCount = 0;
bool FolderOperation::m_FailedToWatch = false;
std::mutex FolderOperation::m_CbMutex;

/**************************************************************************************************/
void FolderOperation::DirectoryWatcherCallback(string directory, CallbackType type, string error)
{
	std::unique_lock<std::mutex> lock(FolderOperation::m_CbMutex);
	if (type == FailedToWatch)
	{
		FolderOperation::m_FailedToWatch = true;
		cout << error << endl;
	}

	FolderOperation::m_CallbackCount++;
	lock.unlock();
}


unsigned FolderOperation::CallbackCount()
{
	std::lock_guard<std::mutex> lock(m_CbMutex);
	return FolderOperation::m_CallbackCount;
}

bool FolderOperation::WatcherFailed()
{
	std::lock_guard<std::mutex> lock(m_CbMutex);
	return FolderOperation::m_FailedToWatch;
}

/**************************************************************************************************/
bool CreateDirectoriesRecursive(int iteration, int maxIterations, CHAR rootDir[], string lastPath)
{
	char chars[] = "ABC";

	for (int i = 0; i < 3; i++)
	{
		string tmpPath = lastPath + chars[i] + "\\";
		string dirPath = rootDir + tmpPath;

		if (iteration <= maxIterations)
		{
			if (SHCreateDirectoryEx(NULL, dirPath.c_str(), NULL) == ERROR_SUCCESS)
			{
					CreateDirectoriesRecursive(iteration + 1, maxIterations, rootDir, tmpPath);
			}
			else
				return false;
		}
		else
			return true;
	}
}

/**************************************************************************************************/
LONG FolderOperation::DeleteDirectoryAndAllSubfolders(string wzDirectory)
{
	CHAR szDir[MAX_PATH + 1];
	SHFILEOPSTRUCT fos = { 0 };

	StringCchCopy(szDir, MAX_PATH, wzDirectory.c_str());
	int len = lstrlen(szDir);
	szDir[len + 1] = 0;

	fos.wFunc = FO_DELETE;
	fos.pFrom = szDir;
	fos.fFlags = FOF_SILENT | FOF_NOCONFIRMATION | FOF_NOERRORUI | FOF_NOCONFIRMMKDIR;
	return SHFileOperation(&fos);
}

/**************************************************************************************************/
/*                                                                                                */
/**************************************************************************************************/
FolderOperation::FolderOperation()
{
	m_CallbackCount = 0;
	m_FailedToWatch = false;
	m_pathToTemp[MAX_PATH];
	GetTempPath(MAX_PATH, m_pathToTemp);
};

/**************************************************************************************************/
FolderOperation::~FolderOperation() {
	DeleteFolder();
};

/**************************************************************************************************/
void FolderOperation::SetUp()
{
	CreateFolder();
}

/**************************************************************************************************/
void FolderOperation::CreateFolder()
{
	CreateDirectoriesRecursive(1, 4, m_pathToTemp, "");
	std::cout << "Test folders created..." << std::endl;
}

/**************************************************************************************************/
void FolderOperation::DeleteFolder()
{
	char chars[] = "ABC";

	for (int i = 0; i < 3; i++)
	{
		DeleteDirectoryAndAllSubfolders(string(m_pathToTemp) + "\\" + chars[i] + "\\");
	}
	std::cout << "Clean up..." << std::endl;
}