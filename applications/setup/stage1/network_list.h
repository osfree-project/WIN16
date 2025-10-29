#ifndef NETWORK_LIST_H
#define NETWORK_LIST_H

#define MAX_VDD_COUNT 10  /* Максимальное количество VDD драйверов */

typedef struct NetworkEntry {
    char far *name;
    char far *driver_file;
    char far *description;
    char far *help_file;
    char far *opt_file;
    char far *winini_sect_name;
    char far *sysini_sect_name;
    char far *vdd_list[MAX_VDD_COUNT];  /* Массив для VDD драйверов */
    int vdd_count;                      /* Фактическое количество VDD драйверов */
    struct NetworkEntry *next;
} NetworkEntry;

/* Network list operations */
void network_list_init(void);
void network_list_add(const char* name, const char* driver_file, const char* description,
                     const char* help_file, const char* opt_file, const char* winini_sect_name,
                     const char* sysini_sect_name, char* vdd_array[], int vdd_count);
NetworkEntry* network_list_find(const char* name);
int network_list_count(void);
void network_list_clear(void);
void network_list_free(void);

#endif
