#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include "kbdtable_list.h"

static KbdTableEntry *kbdtable_list_head = NULL;
static int kbdtable_list_size = 0;

void kbdtable_list_init(void) {
    kbdtable_list_head = NULL;
    kbdtable_list_size = 0;
}

void kbdtable_list_add(const char* name, const char* dll, const char* description) {
    KbdTableEntry *new_entry;
    unsigned int name_len, dll_len, desc_len;
    
    if (!name) return;
    
    new_entry = (KbdTableEntry*)malloc(sizeof(KbdTableEntry));
    if (!new_entry) return;
    
    memset(new_entry, 0, sizeof(KbdTableEntry));
    
    /* Allocate and copy name */
    name_len = strlen(name);
    new_entry->name = (char far*)_fmalloc(name_len + 1);
    if (new_entry->name) {
        _fstrcpy(new_entry->name, name);
    }
    
    /* Allocate and copy DLL if present */
    if (dll && dll[0] != '\0') {
        dll_len = strlen(dll);
        new_entry->dll = (char far*)_fmalloc(dll_len + 1);
        if (new_entry->dll) {
            _fstrcpy(new_entry->dll, dll);
        }
    }
    
    /* Allocate and copy description if present */
    if (description && description[0] != '\0') {
        desc_len = strlen(description);
        new_entry->description = (char far*)_fmalloc(desc_len + 1);
        if (new_entry->description) {
            _fstrcpy(new_entry->description, description);
        }
    }
    
    /* Add to linked list */
    new_entry->next = kbdtable_list_head;
    kbdtable_list_head = new_entry;
    kbdtable_list_size++;
}

KbdTableEntry* kbdtable_list_find(const char* name) {
    KbdTableEntry *current = kbdtable_list_head;
    
    while (current) {
        if (current->name && _fstrcmp(current->name, name) == 0) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

int kbdtable_list_count(void) {
    return kbdtable_list_size;
}

void kbdtable_list_clear(void) {
    KbdTableEntry *current = kbdtable_list_head;
    KbdTableEntry *next;
    
    while (current) {
        next = current->next;
        if (current->name) _ffree(current->name);
        if (current->dll) _ffree(current->dll);
        if (current->description) _ffree(current->description);
        free(current);
        current = next;
    }
    
    kbdtable_list_head = NULL;
    kbdtable_list_size = 0;
}

void kbdtable_list_free(void) {
    kbdtable_list_clear();
}
