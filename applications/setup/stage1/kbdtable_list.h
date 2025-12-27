#ifndef KBDTABLE_LIST_H
#define KBDTABLE_LIST_H

typedef struct KbdTableEntry {
    char far *name;
    char far *dll;
    char far *description;
    struct KbdTableEntry *next;
} KbdTableEntry;

/* Keyboard table list operations */
void kbdtable_list_init(void);
void kbdtable_list_add(const char* name, const char* dll, const char* description);
KbdTableEntry* kbdtable_list_find(const char* name);
int kbdtable_list_count(void);
void kbdtable_list_clear(void);
void kbdtable_list_free(void);

#endif
