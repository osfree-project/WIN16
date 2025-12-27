#ifndef FILE_UTILS_H
#define FILE_UTILS_H

#include <stdio.h>
#include <sys/stat.h>

#define MAX_PATH 80

int is_source_file_newer(const char *source, const char *dest);
int file_exists(const char *filename);
long file_size(const char *filename);
long get_free_disk_space(const char *path);
void create_directory(const char *path);
int copy_file(const char *source, const char *dest);
int is_compressed_file(const char *filename);
int expand_file(const char *source, const char *dest);

#endif
