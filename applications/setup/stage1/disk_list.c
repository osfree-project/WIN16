#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <malloc.h>
#include "disk_list.h"
#include "log.h"

static DiskInfo *disk_list_head = NULL;

void disk_list_add(int disk, const char* path, const char* label, const char* tag) {
    DiskInfo *new_disk;
    
//    log_message("disk_list_add: Adding disk %d: path='%s', label='%s', tag='%s'", 
//                disk, path, label, tag);
    
    /* Create new disk entry */
    new_disk = (DiskInfo *)malloc(sizeof(DiskInfo));
    if (!new_disk) {
        log_message("disk_list_add: ERROR - memory allocation failed");
        return;
    }
    
    new_disk->disk = disk;
    
    /* Allocate far memory for strings and copy data */
    if (path) {
        new_disk->path = (char far*)_fmalloc(strlen(path) + 1);
        if (new_disk->path) {
            _fstrcpy(new_disk->path, path);
        }
    } else {
        new_disk->path = NULL;
    }
    
    if (label) {
        new_disk->label = (char far*)_fmalloc(strlen(label) + 1);
        if (new_disk->label) {
            _fstrcpy(new_disk->label, label);
        }
    } else {
        new_disk->label = NULL;
    }
    
    if (tag) {
        new_disk->tag = (char far*)_fmalloc(strlen(tag) + 1);
        if (new_disk->tag) {
            _fstrcpy(new_disk->tag, tag);
        }
    } else {
        new_disk->tag = NULL;
    }
    
    /* Add to linked list */
    new_disk->next = disk_list_head;
    disk_list_head = new_disk;
    
//    log_message("disk_list_add: Disk %d added successfully", disk);
}

DiskInfo* disk_list_find(int disk) {
    DiskInfo *current = disk_list_head;
    
    while (current != NULL) {
        if (current->disk == disk) {
            return current;
        }
        current = current->next;
    }
    
    return NULL;
}

void disk_list_free_all(void) {
    DiskInfo *current = disk_list_head;
    DiskInfo *next;
    int count = 0;
    
//    log_message("disk_list_free_all: Freeing disk list");
    
    while (current != NULL) {
        next = current->next;
        
        /* Free far strings */
        if (current->path) _ffree(current->path);
        if (current->label) _ffree(current->label);
        if (current->tag) _ffree(current->tag);
        
        free(current);
        current = next;
        count++;
    }
    
    disk_list_head = NULL;
//    log_message("disk_list_free_all: Freed %d disk entries", count);
}

char* get_disk_path(int disk) {
    DiskInfo* info;
    char* result;
    
    info = disk_list_find(disk);
    if (info && info->path) {
        unsigned int len = _fstrlen(info->path);
        result = (char*)malloc(len + 1);
        if (result) {
            _fstrncpy(result, info->path, len);
            result[len] = '\0';
        }
        return result;
    }
    return NULL;
}

char* get_disk_label(int disk) {
    DiskInfo* info;
    char* result;
    
    info = disk_list_find(disk);
    if (info && info->label) {
        unsigned int len = _fstrlen(info->label);
        result = (char*)malloc(len + 1);
        if (result) {
            _fstrncpy(result, info->label, len);
            result[len] = '\0';
        }
        return result;
    }
    return NULL;
}

char* get_disk_tag(int disk) {
    DiskInfo* info;
    char* result;
    
    info = disk_list_find(disk);
    if (info && info->tag) {
        unsigned int len = _fstrlen(info->tag);
        result = (char*)malloc(len + 1);
        if (result) {
            _fstrncpy(result, info->tag, len);
            result[len] = '\0';
        }
        return result;
    }
    return NULL;
}
