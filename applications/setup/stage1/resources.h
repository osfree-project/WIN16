#ifndef RESOURCES_H
#define RESOURCES_H

#include <stdio.h>
#include <stdarg.h>

#define MAX_LANGUAGE_LENGTH 10

#define CPU_8086 0
#define CPU_286  1
#define CPU_386  2

typedef struct {
    char far *key;
    char far *value;
    char far *condition;
} StringResource;

char* get_string(const char* section, const char* key);
char* get_string_default(const char* section, const char* key, const char* default_value);
char* get_string_for_cpu(const char* base_section, const char* key, int cpu_type);
int get_int(const char* section, const char* key, int default_value);

char* get_current_language(void);
void free_resources(void);

char* get_private_profile_string(const char* section, const char* key, const char* default_val, const char* filename);
int get_private_profile_int(const char* section, const char* key, int default_val, const char* filename);

#endif
