#ifndef LANGUAGE_LIST_H
#define LANGUAGE_LIST_H

typedef struct LanguageEntry {
    char far *name;
    char far *dll;
    char far *description;
    struct LanguageEntry *next;
} LanguageEntry;

/* Language list operations */
void language_list_init(void);
void language_list_add(const char* name, const char* dll, const char* description);
LanguageEntry* language_list_find(const char* name);
int language_list_count(void);
void language_list_clear(void);
void language_list_free(void);

#endif
