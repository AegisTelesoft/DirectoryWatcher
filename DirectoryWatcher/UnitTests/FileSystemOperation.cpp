#include "FileSystemOperation.h"

unsigned FileSystemOperation::m_CallbackCount = 0;
bool FileSystemOperation::m_FailedToWatch = false;
std::mutex FileSystemOperation::m_CbMutex;

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
				// Creating test file
				HANDLE file = CreateFile(string(dirPath + "\\Temp" + chars[i] + ".txt").c_str(),
					GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
				CloseHandle(file);

				// Continue recursion
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
/*                                                                                                */
/**************************************************************************************************/
void FileSystemOperation::DirectoryWatcherCallback(string directory, CallbackType type, string error)
{
	std::unique_lock<std::mutex> lock(FileSystemOperation::m_CbMutex);
	if (type == FailedToWatch)
	{
		FileSystemOperation::m_FailedToWatch = true;
		cout << error << endl;
	}
	//else
	//{
	//	cout << directory << " " << type << endl;
	//}

	FileSystemOperation::m_CallbackCount++;
	lock.unlock();
}

/**************************************************************************************************/
unsigned FileSystemOperation::CallbackCount()
{
	std::lock_guard<std::mutex> lock(m_CbMutex);
	return FileSystemOperation::m_CallbackCount;
}

/**************************************************************************************************/
bool FileSystemOperation::WatcherFailed()
{
	std::lock_guard<std::mutex> lock(m_CbMutex);
	return FileSystemOperation::m_FailedToWatch;
}

/**************************************************************************************************/
LONG FileSystemOperation::DeleteDirectoryAndAllSubfolders(string wzDirectory)
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
FileSystemOperation::FileSystemOperation()
{
	m_CallbackCount = 0;
	m_FailedToWatch = false;
	m_pathToTemp[MAX_PATH];
	GetTempPath(MAX_PATH, m_pathToTemp);
};

/**************************************************************************************************/
FileSystemOperation::~FileSystemOperation() {
	DeleteFolder();
};

/**************************************************************************************************/
void FileSystemOperation::SetUp()
{
	CreateFolder();
}

/**************************************************************************************************/
void FileSystemOperation::CreateFolder()
{
	CreateDirectoriesRecursive(1, 4, m_pathToTemp, "");
}

/**************************************************************************************************/
void FileSystemOperation::DeleteFolder()
{
	char chars[] = "ABC";

	for (int i = 0; i < 3; i++)
	{
		DeleteDirectoryAndAllSubfolders(string(m_pathToTemp) + "\\" + chars[i] + "\\");
	}
}