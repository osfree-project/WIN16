#ifndef DISPLAY_LIST_H
#define DISPLAY_LIST_H

typedef struct DisplayEntry {
    char far *name;
    char far *driver_file;
    char far *description;
    char far *resolution;
    char far *grabber_286;
    char far *logo_code;
    char far *vdd_file;
    char far *grabber_386;
    char far *ega_sys;
    char far *logo_data;
    struct DisplayEntry *next;
} DisplayEntry;

/* Display list operations */
void display_list_init(void);
void display_list_add(const char* name, const char* driver_file, const char* description,
                     const char* resolution, const char* grabber_286, const char* logo_code,
                     const char* vdd_file, const char* grabber_386, const char* ega_sys,
                     const char* logo_data);
DisplayEntry* display_list_find(const char* name);
int display_list_count(void);
void display_list_free(void);

#endif
