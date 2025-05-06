#ifndef VMAP_H
#define VMAP_H

#include <stddef.h>

#define KB(x) ((size_t)(x) * 1024ULL)
#define MB(x) ((size_t)(x) * 1024ULL * 1024ULL)
#define GB(x) ((size_t)(x) * 1024ULL * 1024ULL * 1024ULL)
#define TB(x) ((size_t)(x) * 1024ULL * 1024ULL * 1024ULL * 1024ULL)

int vmap_reserve(size_t bytes, void** out);
int vmap_commit(size_t bytes, void* block);
int vmap_prefault(size_t bytes, void* block);
int vmap_decommit(size_t bytes, void* block);
int vmap_release(void* block);

#endif // VMAP_H
