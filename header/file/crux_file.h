#ifndef CRUX_FILE_H
#define CRUX_FILE_H

#include <stddef.h>

char* crux_file_load_text(const char* path, size_t* out_size);

#endif // CRUX_FILE_H