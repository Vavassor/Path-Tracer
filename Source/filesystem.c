#include "filesystem.h"

#include <stdio.h>

bool save_whole_file(const char* path, const void* contents, uint64_t bytes)
{
    FILE* file = fopen(path, "wb");
    if(!file)
    {
        return false;
    }

    uint64_t written = fwrite(contents, 1, bytes, file);
    if(written != bytes)
    {
        return false;
    }

    fclose(file);

    return true;
}
