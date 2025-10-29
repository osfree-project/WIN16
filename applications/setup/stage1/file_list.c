#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "log.h"
#include "file_list.h"

static FileCopyEntry *file_list_head = NULL;

void file_list_add_entry(char disk, const char *source_name, const char *dest_dir, const char *dest_name) {
    FileCopyEntry *new_entry;
    FileCopyEntry *current;
    
//    log_message("file_list_add_entry: Adding file copy entry - disk: %d, source: '%s', dest_dir: '%s', dest_name: '%s'", 
//                disk, source_name, dest_dir, dest_name);
    
    /* Create new entry */
    new_entry = (FileCopyEntry *)malloc(sizeof(FileCopyEntry));
    if (!new_entry) {
        log_message("file_list_add_entry: ERROR - memory allocation failed for new entry");
        return;
    }
    
    /* Initialize fields */
    new_entry->disk = disk;
    
    new_entry->source_name = (char *)malloc(strlen(source_name) + 1);
    if (new_entry->source_name) {
        strcpy(new_entry->source_name, source_name);
    } else {
        log_message("file_list_add_entry: ERROR - memory allocation failed for source_name");
        free(new_entry);
        return;
    }
    
    new_entry->dest_dir = (char *)malloc(strlen(dest_dir) + 1);
    if (new_entry->dest_dir) {
        strcpy(new_entry->dest_dir, dest_dir);
    } else {
        log_message("file_list_add_entry: ERROR - memory allocation failed for dest_dir");
        free(new_entry->source_name);
        free(new_entry);
        return;
    }
    
    new_entry->dest_name = (char *)malloc(strlen(dest_name) + 1);
    if (new_entry->dest_name) {
        strcpy(new_entry->dest_name, dest_name);
    } else {
        log_message("file_list_add_entry: ERROR - memory allocation failed for dest_name");
        free(new_entry->source_name);
        free(new_entry->dest_dir);
        free(new_entry);
        return;
    }
    
    new_entry->next = NULL;
    
    /* Add to linked list */
    if (!file_list_head) {
        file_list_head = new_entry;
    } else {
        current = file_list_head;
        while (current->next) {
            current = current->next;
        }
        current->next = new_entry;
    }
    
//    log_message("file_list_add_entry: File copy entry added successfully");
}

FileCopyEntry *file_list_get_entries_by_disk(char disk) {
    FileCopyEntry *current;
    FileCopyEntry *disk_entries = NULL;
    FileCopyEntry *disk_entries_tail = NULL;
    FileCopyEntry *new_disk_entry;
    
//    log_message("file_list_get_entries_by_disk: Getting entries for disk %d", disk);
    
    current = file_list_head;
    while (current) {
        if (current->disk == disk) {
            /* Create a copy of the entry for the disk list */
            new_disk_entry = (FileCopyEntry *)malloc(sizeof(FileCopyEntry));
            if (!new_disk_entry) {
                log_message("file_list_get_entries_by_disk: ERROR - memory allocation failed for disk entry copy");
                file_list_free_entries(disk_entries);
                return NULL;
            }
            
            new_disk_entry->disk = current->disk;
            
            new_disk_entry->source_name = (char *)malloc(strlen(current->source_name) + 1);
            if (new_disk_entry->source_name) {
                strcpy(new_disk_entry->source_name, current->source_name);
            } else {
                log_message("file_list_get_entries_by_disk: ERROR - memory allocation failed for source_name copy");
                free(new_disk_entry);
                file_list_free_entries(disk_entries);
                return NULL;
            }
            
            new_disk_entry->dest_dir = (char *)malloc(strlen(current->dest_dir) + 1);
            if (new_disk_entry->dest_dir) {
                strcpy(new_disk_entry->dest_dir, current->dest_dir);
            } else {
                log_message("file_list_get_entries_by_disk: ERROR - memory allocation failed for dest_dir copy");
                free(new_disk_entry->source_name);
                free(new_disk_entry);
                file_list_free_entries(disk_entries);
                return NULL;
            }
            
            new_disk_entry->dest_name = (char *)malloc(strlen(current->dest_name) + 1);
            if (new_disk_entry->dest_name) {
                strcpy(new_disk_entry->dest_name, current->dest_name);
            } else {
                log_message("file_list_get_entries_by_disk: ERROR - memory allocation failed for dest_name copy");
                free(new_disk_entry->source_name);
                free(new_disk_entry->dest_dir);
                free(new_disk_entry);
                file_list_free_entries(disk_entries);
                return NULL;
            }
            
            new_disk_entry->next = NULL;
            
            /* Add to disk entries list */
            if (!disk_entries) {
                disk_entries = new_disk_entry;
                disk_entries_tail = new_disk_entry;
            } else {
                disk_entries_tail->next = new_disk_entry;
                disk_entries_tail = new_disk_entry;
            }
        }
        current = current->next;
    }
    
//    log_message("file_list_get_entries_by_disk: Found entries for disk %d", disk);
    
    return disk_entries;
}

int file_list_get_unique_disks(char *disks, int max_disks) {
    FileCopyEntry *current;
    int disk_count = 0;
    int i;
    int found;
    
//    log_message("file_list_get_unique_disks: Getting unique disks list");
    
    if (!disks || max_disks <= 0) {
        return 0;
    }
    
    current = file_list_head;
    while (current && disk_count < max_disks) {
        found = 0;
        
        /* Check if disk already in list */
        for (i = 0; i < disk_count; i++) {
            if (disks[i] == current->disk) {
                found = 1;
                break;
            }
        }
        
        if (!found) {
            disks[disk_count] = current->disk;
            disk_count++;
        }
        
        current = current->next;
    }
    
//    log_message("file_list_get_unique_disks: Found %d unique disks", disk_count);
    return disk_count;
}

void file_list_dump(void) {
    FileCopyEntry *current;
    int count = 0;
    
//    log_message("file_list_dump: Dumping file list contents");
    
    current = file_list_head;
    while (current) {
        log_message("file_list_dump: Entry %d - disk: %d, source: '%s', dest_dir: '%s', dest_name: '%s'", 
                   count, current->disk, current->source_name, current->dest_dir, current->dest_name);
        count++;
        current = current->next;
    }
    
//    log_message("file_list_dump: Total %d entries in file list", count);
}

void file_list_free_all(void) {
    FileCopyEntry *current;
    FileCopyEntry *next;
    int count = 0;
    
//    log_message("file_list_free_all: Freeing all file copy entries");
    
    current = file_list_head;
    while (current) {
        next = current->next;
        
        if (current->source_name) free(current->source_name);
        if (current->dest_dir) free(current->dest_dir);
        if (current->dest_name) free(current->dest_name);
        free(current);
        
        count++;
        current = next;
    }
    
    file_list_head = NULL;
//    log_message("file_list_free_all: Freed %d file copy entries", count);
}

void file_list_free_entries(FileCopyEntry *entries) {
    FileCopyEntry *current;
    FileCopyEntry *next;
    int count = 0;
    
//    log_message("file_list_free_entries: Freeing file copy entries list");
    
    current = entries;
    while (current) {
        next = current->next;
        
        if (current->source_name) free(current->source_name);
        if (current->dest_dir) free(current->dest_dir);
        if (current->dest_name) free(current->dest_name);
        free(current);
        
        count++;
        current = next;
    }
    
//    log_message("file_list_free_entries: Freed %d file copy entries", count);
}
