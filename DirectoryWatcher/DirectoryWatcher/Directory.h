#pragma once

#include <string>

using std::string;

class Directory
{
public:
	Directory(const char* path, unsigned id);
	~Directory();
	string GetPath();
	unsigned getId();

private:
	string m_path;
	unsigned m_id;
};