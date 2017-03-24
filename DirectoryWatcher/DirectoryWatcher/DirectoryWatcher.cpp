#include "DirectoryWatcher.h"

DirectoryWatcher::DirectoryWatcher() {
	
}

DirectoryWatcher::DirectoryWatcher(string& directory) {
	m_directories.push_back(Directory(directory, m_directories.size()));
}

DirectoryWatcher::DirectoryWatcher(vector<string>& directories) {
	for (int i = 0; i < directories.size(); i++) {
		m_directories.push_back(Directory(directories[i], m_directories.size() + i));
	}
}

DirectoryWatcher::~DirectoryWatcher() {

}

void DirectoryWatcher::AddDirectory(const char* directory) {

}

void DirectoryWatcher::RemoveDirectory(int id) {

}

void DirectoryWatcher::RemoveDirectory(const char* directory) {

}

void DirectoryWatcher::Watch(bool watchSubDir) {

}

void DirectoryWatcher::Stop() {

}