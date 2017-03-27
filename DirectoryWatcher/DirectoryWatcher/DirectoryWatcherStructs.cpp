#include "DirectoryWatcherStructs.h"

Directory::Directory(string path, int id)
	: Path(path), Id(id) {
}

MasterThreadData::MasterThreadData(vector<Directory>* directories, CancelationToken* token)
	: directories(directories), token(token) {
}

WorkerThreadData::WorkerThreadData(Directory dir, int threadId, CancelationToken* token)
	: directory(dir), threadId(threadId), token(token) {
}
