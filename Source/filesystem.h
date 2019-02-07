#ifndef FILESYSTEM_H_
#define FILESYSTEM_H_

#include <stdbool.h>
#include <stdint.h>

bool save_whole_file(const char* path, const void* contents, uint64_t bytes);

#endif // FILESYSTEM_H_
