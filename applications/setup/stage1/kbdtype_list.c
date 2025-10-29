#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <malloc.h>
#include "kbdtype_list.h"
#include "log.h"

static KbdTypeEntry* kbdtype_list_head = NULL;
static int kbdtype_list_size = 0;

void kbdtype_list_init(void) {
    kbdtype_list_clear();
}

void kbdtype_list_add(const char* name, const char* description, const char* dll) {
    KbdTypeEntry* new_entry;
    unsigned int name_len, desc_len, dll_len;
    
    if (!name) return;
    
    new_entry = (KbdTypeEntry*)malloc(sizeof(KbdTypeEntry));
    if (!new_entry) {
        log_message("kbdtype_list_add: Memory allocation failed for new entry");
        return;
    }
    
    memset(new_entry, 0, sizeof(KbdTypeEntry));
    
    /* Allocate and copy name */
    name_len = strlen(name);
    new_entry->name = (char far*)_fmalloc(name_len + 1);
    if (new_entry->name) {
        _fstrcpy(new_entry->name, name);
    }
    
    /* Allocate and copy description */
    if (description) {
        desc_len = strlen(description);
        new_entry->description = (char far*)_fmalloc(desc_len + 1);
        if (new_entry->description) {
            _fstrcpy(new_entry->description, description);
        }
    }
    
    /* Allocate and copy DLL */
    if (dll) {
        dll_len = strlen(dll);
        new_entry->dll = (char far*)_fmalloc(dll_len + 1);
        if (new_entry->dll) {
            _fstrcpy(new_entry->dll, dll);
        }
    }
    
    /* Add to list */
    new_entry->next = kbdtype_list_head;
    kbdtype_list_head = new_entry;
    kbdtype_list_size++;
    
    log_message("kbdtype_list_add: Added keyboard type '%s'", name);
}

KbdTypeEntry* kbdtype_list_find(const char* name) {
    KbdTypeEntry* current = kbdtype_list_head;
    
    while (current) {
        if (current->name && _fstrcmp(current->name, name) == 0) {
            return current;
        }
        current = current->next;
    }
    
    return NULL;
}

int kbdtype_list_count(void) {
    return kbdtype_list_size;
}

void kbdtype_list_clear(void) {
    KbdTypeEntry* current = kbdtype_list_head;
    KbdTypeEntry* next;
    
    while (current) {
        next = current->next;
        
        if (current->name) _ffree(current->name);
        if (current->description) _ffree(current->description);
        if (current->dll) _ffree(current->dll);
        
        free(current);
        current = next;
    }
    
    kbdtype_list_head = NULL;
    kbdtype_list_size = 0;
    
    log_message("kbdtype_list_clear: Keyboard type list cleared");
}

void kbdtype_list_free(void) {
    kbdtype_list_clear();
}
