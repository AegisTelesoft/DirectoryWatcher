#pragma once

#include <windows.h>

struct overlapped_struct;

class DirInfo
{
public:
	DirInfo(int bufferSize);
	~DirInfo();

	wchar_t* Buffer;
	wchar_t* DirectoryName;
	ULONG_PTR CompletionKey;

	HANDLE IoCompletionPortHandle;
	HANDLE DirEventHandle;
	int BufferLength;
	overlapped_struct* Overlapped;
private:
	void Init(int bufferSize);
};

struct overlapped_struct
{
	OVERLAPPED Overlapped;
	wchar_t* Buffer;
};
