#ifndef MOUSE_LIST_H
#define MOUSE_LIST_H

typedef struct MouseEntry {
    char far *name;
    char far *driver_file;
    char far *description;
    char far *vmd;
    char far *dos_driver;
    struct MouseEntry *next;
} MouseEntry;

/* Mouse list operations */
void mouse_list_init(void);
void mouse_list_add(const char* name, const char* driver_file, const char* description,
                   const char* vmd, const char* dos_driver);
MouseEntry* mouse_list_find(const char* name);
int mouse_list_count(void);
void mouse_list_clear(void);
void mouse_list_free(void);

#endif
