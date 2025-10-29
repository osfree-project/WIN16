#ifndef KBDTYPE_LIST_H
#define KBDTYPE_LIST_H

typedef struct KbdTypeEntry {
    char far *name;
    char far *description;
    char far *dll;
    struct KbdTypeEntry *next;
} KbdTypeEntry;

/* Keyboard type list operations */
void kbdtype_list_init(void);
void kbdtype_list_add(const char* name, const char* description, const char* dll);
KbdTypeEntry* kbdtype_list_find(const char* name);
int kbdtype_list_count(void);
void kbdtype_list_clear(void);
void kbdtype_list_free(void);

#endif
