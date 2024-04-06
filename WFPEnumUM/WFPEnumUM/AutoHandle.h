#pragma once
#include <Windows.h>

class AutoHandle
{
private:
	HANDLE _handle;
public:
	AutoHandle(HANDLE handle)
	{
		_handle = handle;
	};
	~AutoHandle()
	{
		CloseHandle(_handle);
	};
};