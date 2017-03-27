#pragma once

#include <string>
#include <vector>

#include "CancelationToken.h"

using std::string;
using std::vector;

struct Directory
{
	Directory(string path, int id);

	string Path;
	int Id;
};

struct WorkerThreadData
{
	WorkerThreadData(Directory dir, int threadId, CancelationToken* token);

	struct Directory directory;
	CancelationToken* token;
	int threadId;
};

struct MasterThreadData {
	MasterThreadData(vector<Directory>* directories, CancelationToken* token);

	vector<Directory>* directories;
	CancelationToken* token;
};