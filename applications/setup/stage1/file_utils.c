#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <direct.h>
#include <fcntl.h>
#include <io.h>
#include <sys/stat.h>
#include <malloc.h>
#include <dos.h>
#include "file_utils.h"
#include "log.h"

int is_source_file_newer(const char *source, const char *dest) {
    struct stat source_stat, dest_stat;
    
    if (stat(source, &source_stat) != 0) {
        return 1; /* Source doesn't exist? Shouldn't happen, but copy anyway */
    }
    
    if (stat(dest, &dest_stat) != 0) {
        return 1; /* Destination doesn't exist, definitely copy */
    }
    
    /* Compare modification times */
    if (source_stat.st_mtime > dest_stat.st_mtime) {
        return 1; /* Source is newer */
    }
    
    return 0; /* Source is older or same */
}

int file_exists(const char *filename) {
    FILE *file = fopen(filename, "rb");
    if (file) {
        fclose(file);
        return 1;
    }
    return 0;
}

long file_size(const char *filename) {
    struct stat st;
    if (stat(filename, &st) == 0) {
        return st.st_size;
    }
    return -1;
}

long get_free_disk_space(const char *path) {
    struct diskfree_t df;
    unsigned drive;
    
    if (path == NULL || strlen(path) < 2) return -1;
    
    drive = toupper(path[0]) - 'A' + 1;
    
    if (_dos_getdiskfree(drive, &df) == 0) {
        return (long)df.avail_clusters * (long)df.bytes_per_sector * (long)df.sectors_per_cluster;
    }
    return -1;
}

void create_directory(const char *path) {
    char tmp[MAX_PATH];
    char *p;
    
    if (path == NULL || strlen(path) == 0) return;
    
    strncpy(tmp, path, sizeof(tmp)-1);
    tmp[sizeof(tmp)-1] = '\0';
    
    /* Skip drive letter */
    p = tmp + 2;
    
    while (*p) {
        if (*p == '\\' || *p == '/') {
            char save_char = *p;
            *p = '\0';
            mkdir(tmp);
            *p = save_char;
        }
        p++;
    }
    mkdir(tmp);
}

int copy_file(const char *source, const char *dest) {
    FILE *src, *dst;
    char buffer[2048];
    size_t bytes;
    char destDir[MAX_PATH];
    char *lastSlash;
    
    if (source == NULL || dest == NULL) return 0;
    
    log_message("Copying file: %s -> %s", source, dest);
    
    src = fopen(source, "rb");
    if (!src) {
        log_message("Error: Cannot open source file %s", source);
        return 0;
    }
    
    strncpy(destDir, dest, sizeof(destDir)-1);
    destDir[sizeof(destDir)-1] = '\0';
    lastSlash = strrchr(destDir, '\\');
    if (lastSlash) {
        *lastSlash = '\0';
        create_directory(destDir);
    }
    
    dst = fopen(dest, "wb");
    if (!dst) {
        log_message("Error: Cannot open destination file %s", dest);
        fclose(src);
        return 0;
    }
    
    while ((bytes = fread(buffer, 1, sizeof(buffer), src)) > 0) {
        if (fwrite(buffer, 1, bytes, dst) != bytes) {
            log_message("Error: Write failed for file %s", dest);
            fclose(src);
            fclose(dst);
            return 0;
        }
    }
    
    fclose(src);
    fclose(dst);
    log_message("Successfully copied: %s", source);
    return 1;
}

int is_compressed_file(const char *filename) {
    FILE *file;
    unsigned char sig[4];
    
    if (filename == NULL) return 0;
    
    file = fopen(filename, "rb");
    if (!file) return 0;
    
    if (fread(sig, 1, 4, file) != 4) {
        fclose(file);
        return 0;
    }
    fclose(file);
    
    return (sig[0] == 'S' && sig[1] == 'Z' && sig[2] == 'D' && sig[3] == 0x88);
}

int expand_file(const char *source, const char *dest) {
    char cmd[512];
    int result;
    
    if (source == NULL || dest == NULL) return 0;
    
    log_message("Expanding compressed file: %s -> %s", source, dest);
    
    if (!is_compressed_file(source)) {
        return copy_file(source, dest);
    }
    
    sprintf(cmd, "expand %s %s", source, dest);
    result = system(cmd);
    
    if (result != 0) {
        log_message("Expand command failed, falling back to copy");
        return copy_file(source, dest);
    }
    
    log_message("Expand command successful");
    return 1;
}
