#include "DirInfo.h"

DirInfo::DirInfo(int bufferSize)
{
	Overlapped = new overlapped_struct;
	Init(bufferSize);
}

void DirInfo::Init(int bufferSize)
{
	Buffer = (wchar_t*)malloc(bufferSize * sizeof(wchar_t));
	BufferLength = bufferSize;
	Overlapped = (overlapped_struct*)calloc(1, sizeof(overlapped_struct));
	Overlapped->Buffer = (wchar_t*)calloc(bufferSize, sizeof(wchar_t));
	memset(Overlapped, 0, sizeof(OVERLAPPED));
	memset(Overlapped->Buffer, 0, bufferSize);
}

DirInfo::~DirInfo()
{
	free(Buffer);
	free(Overlapped->Buffer);
	free(Overlapped);
}