#ifndef DISK_LIST_H
#define DISK_LIST_H

typedef struct DiskInfo {
    int disk;
    char far *path;
    char far *label;
    char far *tag;
    struct DiskInfo *next;
} DiskInfo;

/* Disk list operations */
void disk_list_add(int disk, const char* path, const char* label, const char* tag);
DiskInfo* disk_list_find(int disk);
void disk_list_free_all(void);

/* Disk information getters */
char* get_disk_path(int disk);
char* get_disk_label(int disk);
char* get_disk_tag(int disk);

#endif
