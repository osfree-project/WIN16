#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <malloc.h>
#include "mouse_list.h"
#include "log.h"

static MouseEntry *mouse_list_head = NULL;
static int mouse_list_count_value = 0;

void mouse_list_init(void) {
    mouse_list_clear();
}

void mouse_list_add(const char* name, const char* driver_file, const char* description,
                   const char* vmd, const char* dos_driver) {
    MouseEntry *new_entry, *current;
    
    if (!name || !driver_file || !description) {
        log_message("mouse_list_add: ERROR - null parameters");
        return;
    }
    
    new_entry = (MouseEntry*)malloc(sizeof(MouseEntry));
    if (!new_entry) {
        log_message("mouse_list_add: ERROR - memory allocation failed");
        return;
    }
    
    /* Initialize the new entry */
    memset(new_entry, 0, sizeof(MouseEntry));
    
    /* Allocate far memory for strings and copy data */
    new_entry->name = (char far*)_fmalloc(strlen(name) + 1);
    if (new_entry->name) {
        _fstrcpy(new_entry->name, name);
    }
    
    new_entry->driver_file = (char far*)_fmalloc(strlen(driver_file) + 1);
    if (new_entry->driver_file) {
        _fstrcpy(new_entry->driver_file, driver_file);
    }
    
    new_entry->description = (char far*)_fmalloc(strlen(description) + 1);
    if (new_entry->description) {
        _fstrcpy(new_entry->description, description);
    }
    
    if (vmd && vmd[0] != '\0') {
        new_entry->vmd = (char far*)_fmalloc(strlen(vmd) + 1);
        if (new_entry->vmd) {
            _fstrcpy(new_entry->vmd, vmd);
        }
    } else {
        new_entry->vmd = NULL;
    }
    
    if (dos_driver && dos_driver[0] != '\0') {
        new_entry->dos_driver = (char far*)_fmalloc(strlen(dos_driver) + 1);
        if (new_entry->dos_driver) {
            _fstrcpy(new_entry->dos_driver, dos_driver);
        }
    } else {
        new_entry->dos_driver = NULL;
    }
    
    new_entry->next = NULL;
    
    /* Add to the end of the list */
    if (!mouse_list_head) {
        mouse_list_head = new_entry;
    } else {
        current = mouse_list_head;
        while (current->next) {
            current = current->next;
        }
        current->next = new_entry;
    }
    
    mouse_list_count_value++;
    
    log_message("mouse_list_add: Added mouse '%s' - driver: '%s', desc: '%s', vmd: '%s', dos: '%s'", 
                name, driver_file, description, vmd ? vmd : "NULL", dos_driver ? dos_driver : "NULL");
}

MouseEntry* mouse_list_find(const char* name) {
    MouseEntry *current = mouse_list_head;
    
    if (!name) return NULL;
    
    while (current) {
        if (current->name && _fstrcmp(current->name, (const char far *)name) == 0) {
            return current;
        }
        current = current->next;
    }
    
    return NULL;
}

int mouse_list_count(void) {
    return mouse_list_count_value;
}

void mouse_list_clear(void) {
    MouseEntry *current = mouse_list_head;
    MouseEntry *next;
    
    while (current) {
        next = current->next;
        
        /* Free all far strings */
        if (current->name) _ffree(current->name);
        if (current->driver_file) _ffree(current->driver_file);
        if (current->description) _ffree(current->description);
        if (current->vmd) _ffree(current->vmd);
        if (current->dos_driver) _ffree(current->dos_driver);
        
        free(current);
        current = next;
    }
    
    mouse_list_head = NULL;
    mouse_list_count_value = 0;
    
    log_message("mouse_list_clear: Mouse list cleared");
}

void mouse_list_free(void) {
    mouse_list_clear();
}
