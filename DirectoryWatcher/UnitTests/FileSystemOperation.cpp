#include "FileSystemOperation.h"

#include <string>
#include <mutex>
#include "Shlwapi.h"
#include "Shlobj.h"
#include "Strsafe.h"

using std::string;

/**************************************************************************************************/
void CreateTestFiles(string rootDir)
{
	string dirPath = rootDir;
	
	char chars[] = "ABC";
	for (int i = 0; i < 3; i++)
	{
		dirPath = dirPath + chars[i] + "\\";

		if (SHCreateDirectoryEx(NULL, dirPath.c_str(), NULL) == ERROR_SUCCESS)
		{
			// Creating test file
			string filePath = dirPath + "\\Temp" + chars[i] + ".txt";

			HANDLE file = CreateFile(filePath.c_str(),
				GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
			CloseHandle(file);
		}
	}
}

/**************************************************************************************************/
/*                                                                                                */
/**************************************************************************************************/
LONG FileSystemOperation::DeleteDirectoryAndAllSubfolders(string directory)
{
	CHAR szDir[MAX_PATH + 1];
	SHFILEOPSTRUCT fos = { 0 };

	StringCchCopy(szDir, MAX_PATH, directory.c_str());
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
	CHAR path[MAX_PATH];
	GetTempPath(MAX_PATH, path);
	m_pathToTemp = move(string(path));
};

/**************************************************************************************************/
FileSystemOperation::~FileSystemOperation() 
{
	DeleteDirectoryAndAllSubfolders(m_pathToTemp + "\\A\\");
};

/**************************************************************************************************/
void FileSystemOperation::SetUp()
{
	CreateTestFiles(m_pathToTemp);
}

/**************************************************************************************************/
string FileSystemOperation::GetTestDirPath()
{
	return m_pathToTemp;
}
