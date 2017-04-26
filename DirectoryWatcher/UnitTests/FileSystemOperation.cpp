#include "FileSystemOperation.h"

#include <string>

#if defined (WINDOWS)
	#include <Windows.h>
	#include "Shlwapi.h"
	#include "Shlobj.h"
	#include "Strsafe.h"
#elif defined (LINUX)
	#include <sys/types.h>
	#include <sys/stat.h>
	#include <unistd.h>
	#include <fcntl.h>
	#include <ftw.h>
#endif


using std::string;

/**************************************************************************************************/
void CreateTestFiles(string rootDir)
{
	string dirPath = rootDir;
	
	char chars[] = "ABC";
	for (int i = 0; i < 3; i++)
	{
	#if defined WINDOWS

		dirPath = dirPath + chars[i] + "\\";

		if (SHCreateDirectoryEx(NULL, dirPath.c_str(), NULL) == ERROR_SUCCESS)
		{
			// Creating test file
			string filePath = dirPath + "\\Temp" + chars[i] + ".txt";

			HANDLE file = CreateFile(filePath.c_str(),
				GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
			CloseHandle(file);
		}

	#elif defined LINUX

		dirPath = dirPath + chars[i] + "/";

		struct stat st = {0};

		if (stat(dirPath.c_str(), &st) == -1) {
			mkdir(dirPath.c_str(), 0700);
		}

		mode_t perms = S_IRWXU;
		int fd = open((dirPath + "Temp" + chars[i] + ".txt").c_str(), O_CREAT|O_EXCL, perms);

	#endif
	}
}

/**************************************************************************************************/
/*                                                                                                */
/**************************************************************************************************/
#ifdef LINUX

int delete_cb(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf)
{
	int success = remove(fpath);

	if (success)
		perror(fpath);

	return success;
}

int delete_recursively(const char* path)
{
	return nftw(path, delete_cb, 64, FTW_DEPTH | FTW_PHYS);
}

#endif

void FileSystemOperation::DeleteDirectoryAndAllSubfolders(string directory)
{
#if defined WINDOWS
	CHAR szDir[MAX_PATH + 1];
	SHFILEOPSTRUCT fos = { 0 };

	StringCchCopy(szDir, MAX_PATH, directory.c_str());
	int len = lstrlen(szDir);
	szDir[len + 1] = 0;

	fos.wFunc = FO_DELETE;
	fos.pFrom = szDir;
	fos.fFlags = FOF_SILENT | FOF_NOCONFIRMATION | FOF_NOERRORUI | FOF_NOCONFIRMMKDIR;
	SHFileOperation(&fos);

#elif defined LINUX
	delete_recursively(directory.c_str());
#endif
}


/**************************************************************************************************/
FileSystemOperation::FileSystemOperation()
{
#if defined WINDOWS
	CHAR path[MAX_PATH];
	GetTempPath(MAX_PATH, path);

#elif defined LINUX
	const char* path = "/tmp/";
#endif
	m_pathToTemp = move(string(path));
};

/**************************************************************************************************/
FileSystemOperation::~FileSystemOperation() 
{
#if defined WINDOWS
string path = m_pathToTemp + "\\A\\";

#elif defined LINUX
string path = m_pathToTemp + "/A/";

#endif
	
	DeleteDirectoryAndAllSubfolders(path);
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
