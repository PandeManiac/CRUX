#include "file/crux_file.h"
#include "vmap/vmap.h"

#include <stdio.h>
#include <stdint.h>
#include <windows.h>

char* crux_file_load_text(const char* path, size_t* out_size)
{
	HANDLE file = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (file == INVALID_HANDLE_VALUE) return NULL;

	LARGE_INTEGER file_size;

	if (!GetFileSizeEx(file, &file_size))
	{
		CloseHandle(file);
		return NULL;
	}

	size_t size	 = (size_t)file_size.QuadPart;
	size_t total = size + 1;

	char* data = NULL;

	if (vmap_reserve(total, (void**)&data) != 0 || vmap_commit(total, data) != 0 || vmap_prefault(total, data) != 0)
	{
		CloseHandle(file);
		return NULL;
	}

	DWORD read = 0;

	if (!ReadFile(file, data, (DWORD)size, &read, NULL) || read != size)
	{
		CloseHandle(file);
		return NULL;
	}

	data[size] = '\0';
	if (out_size) *out_size = size;
	CloseHandle(file);
	return data;
}