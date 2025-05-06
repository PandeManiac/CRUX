#include "vmap/vmap.h"

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

int vmap_reserve(size_t bytes, void** out)
{
	if (!out || bytes == 0) return ERROR_INVALID_PARAMETER;

	void* reserved = VirtualAlloc(NULL, bytes, MEM_RESERVE, PAGE_READWRITE);
	if (!reserved) return (int)GetLastError();

	*out = reserved;
	return 0;
}

int vmap_commit(size_t bytes, void* block)
{
	if (!block || bytes == 0) return ERROR_INVALID_PARAMETER;

	void* committed = VirtualAlloc(block, bytes, MEM_COMMIT, PAGE_READWRITE);
	if (!committed) return (int)GetLastError();

	return 0;
}

int vmap_prefault(size_t bytes, void* block)
{
	if (!block || bytes == 0) return ERROR_INVALID_PARAMETER;

	SYSTEM_INFO info;
	GetSystemInfo(&info);
	size_t page_size = (size_t)info.dwPageSize;

	char*  ptr	   = (char*)block;
	size_t touched = 0;

	while (touched < bytes)
	{
		volatile char* page = ptr + touched;
		*page				= *page;
		touched += page_size;
	}

	return 0;
}

int vmap_decommit(size_t bytes, void* block)
{
	if (!block || bytes == 0) return ERROR_INVALID_PARAMETER;

	BOOL ok = VirtualFree(block, bytes, MEM_DECOMMIT);
	if (!ok) return (int)GetLastError();

	return 0;
}

int vmap_release(void* block)
{
	if (!block) return ERROR_INVALID_PARAMETER;

	BOOL ok = VirtualFree(block, 0, MEM_RELEASE);
	if (!ok) return (int)GetLastError();

	return 0;
}
