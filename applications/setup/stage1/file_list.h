#ifndef FILE_LIST_H
#define FILE_LIST_H

/* File copy entry structure */
typedef struct FileCopyEntry {
    char disk;  /* -1 for full path source */
    char *source_name;
    char *dest_dir; 
    char *dest_name;
    struct FileCopyEntry *next;
} FileCopyEntry;

/* File list operations */
void file_list_add_entry(char disk, const char *source_name, const char *dest_dir, const char *dest_name);
FileCopyEntry *file_list_get_entries_by_disk(char disk);
void file_list_free_all(void);
void file_list_free_entries(FileCopyEntry *entries);

/* Disk management */
int file_list_get_unique_disks(char *disks, int max_disks);

/* Debug functions */
void file_list_dump(void);

#endif
