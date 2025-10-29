#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include "display_list.h"
#include "log.h"

static DisplayEntry *display_list_head = NULL;
static int display_list_size = 0;

void display_list_init(void) {
    display_list_free();
}

void display_list_add(const char* name, const char* driver_file, const char* description,
                     const char* resolution, const char* grabber_286, const char* logo_code,
                     const char* vdd_file, const char* grabber_386, const char* ega_sys,
                     const char* logo_data) {
    DisplayEntry *new_entry;
    DisplayEntry *current;
    
    /* Create new entry */
    new_entry = (DisplayEntry*)malloc(sizeof(DisplayEntry));
    if (!new_entry) {
        log_message("display_list_add: Memory allocation failed");
        return;
    }
    
    /* Initialize fields with far strings */
    new_entry->name = (char far*)_fmalloc(strlen(name) + 1);
    if (new_entry->name) _fstrcpy(new_entry->name, name);
    
    new_entry->driver_file = (char far*)_fmalloc(strlen(driver_file) + 1);
    if (new_entry->driver_file) _fstrcpy(new_entry->driver_file, driver_file);
    
    new_entry->description = (char far*)_fmalloc(strlen(description) + 1);
    if (new_entry->description) _fstrcpy(new_entry->description, description);
    
    new_entry->resolution = (char far*)_fmalloc(strlen(resolution) + 1);
    if (new_entry->resolution) _fstrcpy(new_entry->resolution, resolution);
    
    new_entry->grabber_286 = (char far*)_fmalloc(strlen(grabber_286) + 1);
    if (new_entry->grabber_286) _fstrcpy(new_entry->grabber_286, grabber_286);
    
    new_entry->logo_code = (char far*)_fmalloc(strlen(logo_code) + 1);
    if (new_entry->logo_code) _fstrcpy(new_entry->logo_code, logo_code);
    
    new_entry->vdd_file = (char far*)_fmalloc(strlen(vdd_file) + 1);
    if (new_entry->vdd_file) _fstrcpy(new_entry->vdd_file, vdd_file);
    
    new_entry->grabber_386 = (char far*)_fmalloc(strlen(grabber_386) + 1);
    if (new_entry->grabber_386) _fstrcpy(new_entry->grabber_386, grabber_386);
    
    new_entry->ega_sys = (char far*)_fmalloc(strlen(ega_sys) + 1);
    if (new_entry->ega_sys) _fstrcpy(new_entry->ega_sys, ega_sys);
    
    new_entry->logo_data = (char far*)_fmalloc(strlen(logo_data) + 1);
    if (new_entry->logo_data) _fstrcpy(new_entry->logo_data, logo_data);
    
    new_entry->next = NULL;
    
    /* Add to list */
    if (display_list_head == NULL) {
        display_list_head = new_entry;
    } else {
        current = display_list_head;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = new_entry;
    }
    
    display_list_size++;
    log_message("display_list_add: Added display '%s', total entries: %d", name, display_list_size);
}

DisplayEntry* display_list_find(const char* name) {
    DisplayEntry *current = display_list_head;
    char near_name[100];
    
    while (current != NULL) {
        if (current->name) {
            /* Convert far string to near for comparison */
            unsigned int len = _fstrlen(current->name);
            if (len >= sizeof(near_name)) len = sizeof(near_name) - 1;
            _fstrncpy(near_name, current->name, len);
            near_name[len] = '\0';
            
            if (strcmp(near_name, name) == 0) {
                return current;
            }
        }
        current = current->next;
    }
    
    return NULL;
}

int display_list_count(void) {
    return display_list_size;
}

void display_list_free(void) {
    DisplayEntry *current = display_list_head;
    DisplayEntry *next;
    
    while (current != NULL) {
        next = current->next;
        
        /* Free all far strings */
        if (current->name) _ffree(current->name);
        if (current->driver_file) _ffree(current->driver_file);
        if (current->description) _ffree(current->description);
        if (current->resolution) _ffree(current->resolution);
        if (current->grabber_286) _ffree(current->grabber_286);
        if (current->logo_code) _ffree(current->logo_code);
        if (current->vdd_file) _ffree(current->vdd_file);
        if (current->grabber_386) _ffree(current->grabber_386);
        if (current->ega_sys) _ffree(current->ega_sys);
        if (current->logo_data) _ffree(current->logo_data);
        
        free(current);
        current = next;
    }
    
    display_list_head = NULL;
    display_list_size = 0;
    log_message("display_list_clear: Display list cleared");
}

