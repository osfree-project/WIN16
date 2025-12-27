#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include "language_list.h"

static LanguageEntry *language_head = NULL;
static int language_count = 0;

void language_list_init(void) {
    language_head = NULL;
    language_count = 0;
}

void language_list_add(const char* name, const char* dll, const char* description) {
    LanguageEntry *new_entry;
    unsigned int name_len, dll_len, desc_len;

    if (!name) return;

    new_entry = (LanguageEntry*)malloc(sizeof(LanguageEntry));
    if (!new_entry) return;

    memset(new_entry, 0, sizeof(LanguageEntry));

    /* Allocate and copy name */
    name_len = strlen(name);
    new_entry->name = (char far*)_fmalloc(name_len + 1);
    if (new_entry->name) {
        _fstrcpy(new_entry->name, name);
    }

    /* Allocate and copy DLL if provided */
    if (dll && dll[0] != '\0') {
        dll_len = strlen(dll);
        new_entry->dll = (char far*)_fmalloc(dll_len + 1);
        if (new_entry->dll) {
            _fstrcpy(new_entry->dll, dll);
        }
    }

    /* Allocate and copy description if provided */
    if (description && description[0] != '\0') {
        desc_len = strlen(description);
        new_entry->description = (char far*)_fmalloc(desc_len + 1);
        if (new_entry->description) {
            _fstrcpy(new_entry->description, description);
        }
    }

    /* Add to list */
    new_entry->next = language_head;
    language_head = new_entry;
    language_count++;
}

LanguageEntry* language_list_find(const char* name) {
    LanguageEntry *current = language_head;

    while (current != NULL) {
        if (current->name && _fstrcmp(current->name, name) == 0) {
            return current;
        }
        current = current->next;
    }

    return NULL;
}

int language_list_count(void) {
    return language_count;
}

void language_list_clear(void) {
    LanguageEntry *current = language_head;
    LanguageEntry *next;

    while (current != NULL) {
        next = current->next;

        if (current->name) _ffree(current->name);
        if (current->dll) _ffree(current->dll);
        if (current->description) _ffree(current->description);

        free(current);
        current = next;
    }

    language_head = NULL;
    language_count = 0;
}

void language_list_free(void) {
    language_list_clear();
}
