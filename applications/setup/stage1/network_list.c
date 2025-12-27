#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include "network_list.h"

static NetworkEntry *network_list_head = NULL;
static int network_list_size = 0;

void network_list_init(void) {
    network_list_head = NULL;
    network_list_size = 0;
}

void network_list_add(const char* name, const char* driver_file, const char* description,
                     const char* help_file, const char* opt_file, const char* winini_sect_name,
                     const char* sysini_sect_name, char* vdd_array[], int vdd_count) {
    NetworkEntry *new_entry, *current;
    int i;
    
    new_entry = (NetworkEntry*)malloc(sizeof(NetworkEntry));
    if (!new_entry) return;
    
    memset(new_entry, 0, sizeof(NetworkEntry));
    
    /* Allocate and copy far strings for each field */
    if (name) {
        new_entry->name = (char far*)_fmalloc(strlen(name) + 1);
        if (new_entry->name) _fstrcpy(new_entry->name, name);
    }
    
    if (driver_file) {
        new_entry->driver_file = (char far*)_fmalloc(strlen(driver_file) + 1);
        if (new_entry->driver_file) _fstrcpy(new_entry->driver_file, driver_file);
    }
    
    if (description) {
        new_entry->description = (char far*)_fmalloc(strlen(description) + 1);
        if (new_entry->description) _fstrcpy(new_entry->description, description);
    }
    
    if (help_file) {
        new_entry->help_file = (char far*)_fmalloc(strlen(help_file) + 1);
        if (new_entry->help_file) _fstrcpy(new_entry->help_file, help_file);
    }
    
    if (opt_file) {
        new_entry->opt_file = (char far*)_fmalloc(strlen(opt_file) + 1);
        if (new_entry->opt_file) _fstrcpy(new_entry->opt_file, opt_file);
    }
    
    if (winini_sect_name) {
        new_entry->winini_sect_name = (char far*)_fmalloc(strlen(winini_sect_name) + 1);
        if (new_entry->winini_sect_name) _fstrcpy(new_entry->winini_sect_name, winini_sect_name);
    }
    
    if (sysini_sect_name) {
        new_entry->sysini_sect_name = (char far*)_fmalloc(strlen(sysini_sect_name) + 1);
        if (new_entry->sysini_sect_name) _fstrcpy(new_entry->sysini_sect_name, sysini_sect_name);
    }
    
    /* Copy VDD drivers from array */
    new_entry->vdd_count = 0;
    if (vdd_array && vdd_count > 0) {
        for (i = 0; i < vdd_count && i < MAX_VDD_COUNT; i++) {
            if (vdd_array[i] && vdd_array[i][0] != '\0') {
                new_entry->vdd_list[i] = (char far*)_fmalloc(strlen(vdd_array[i]) + 1);
                if (new_entry->vdd_list[i]) {
                    _fstrcpy(new_entry->vdd_list[i], vdd_array[i]);
                    new_entry->vdd_count++;
                }
            }
        }
    }
    
    new_entry->next = NULL;
    
    if (network_list_head == NULL) {
        network_list_head = new_entry;
    } else {
        current = network_list_head;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = new_entry;
    }
    
    network_list_size++;
}

NetworkEntry* network_list_find(const char* name) {
    NetworkEntry *current = network_list_head;
    
    while (current != NULL) {
        if (current->name && _fstrcmp(current->name, name) == 0) {
            return current;
        }
        current = current->next;
    }
    
    return NULL;
}

int network_list_count(void) {
    return network_list_size;
}

void network_list_clear(void) {
    NetworkEntry *current = network_list_head;
    NetworkEntry *next;
    int i;
    
    while (current != NULL) {
        next = current->next;
        
        if (current->name) _ffree(current->name);
        if (current->driver_file) _ffree(current->driver_file);
        if (current->description) _ffree(current->description);
        if (current->help_file) _ffree(current->help_file);
        if (current->opt_file) _ffree(current->opt_file);
        if (current->winini_sect_name) _ffree(current->winini_sect_name);
        if (current->sysini_sect_name) _ffree(current->sysini_sect_name);
        
        /* Free VDD drivers */
        for (i = 0; i < current->vdd_count; i++) {
            if (current->vdd_list[i]) _ffree(current->vdd_list[i]);
        }
        
        free(current);
        current = next;
    }
    
    network_list_head = NULL;
    network_list_size = 0;
}

void network_list_free(void) {
    network_list_clear();
}
