#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <malloc.h>
#include "resources.h"
#include "log.h"
#include "inf_parser.h"
#include "section_list.h"
#include "disk_list.h"
#include "machine_list.h"
#include "mouse_list.h"
#include "kbdtype_list.h"
#include "kbdtable_list.h"
#include "language_list.h"  /* NEW: Include language list header */
#include "network_list.h"  /* NEW: Include language list header */

static char current_language[MAX_LANGUAGE_LENGTH] = "usa";

static void parse_section_line(const char* line, char** key, char** value, char** condition) {
    char line_copy[256];
    char *equal_sign;
    char *comma_pos;
    char *temp;
    char *end_ptr;
    char *second_comma;
    
    *key = NULL;
    *value = NULL;
    *condition = NULL;
    
    if (!line || strlen(line) == 0) return;
    
    strncpy(line_copy, line, sizeof(line_copy) - 1);
    line_copy[sizeof(line_copy) - 1] = '\0';
    
    temp = line_copy;
    while (*temp && isspace((unsigned char)*temp)) temp++;
    if (*temp == '\0' || *temp == ';') return;
    
    end_ptr = line_copy + strlen(line_copy) - 1;
    while (end_ptr > line_copy && isspace((unsigned char)*end_ptr)) {
        *end_ptr = '\0';
        end_ptr--;
    }
    
    equal_sign = strchr(line_copy, '=');
    comma_pos = strchr(line_copy, ',');
    
    if (equal_sign) {
        *equal_sign = '\0';
        *key = line_copy;
        *value = equal_sign + 1;
        
        while (**key && isspace((unsigned char)**key)) (*key)++;
        end_ptr = *key + strlen(*key) - 1;
        while (end_ptr > *key && isspace((unsigned char)*end_ptr)) {
            *end_ptr = '\0';
            end_ptr--;
        }
        
        while (**value && isspace((unsigned char)**value)) (*value)++;
        end_ptr = *value + strlen(*value) - 1;
        while (end_ptr > *value && isspace((unsigned char)*end_ptr)) {
            *end_ptr = '\0';
            end_ptr--;
        }
        
        if ((*value)[0] == '"' && (*value)[strlen(*value)-1] == '"') {
            memmove(*value, *value + 1, strlen(*value) - 2);
            (*value)[strlen(*value) - 2] = '\0';
        }
    }
    else if (comma_pos) {
        *comma_pos = '\0';
        *key = line_copy;
        
        second_comma = strchr(comma_pos + 1, ',');
        if (second_comma) {
            *second_comma = '\0';
            *value = comma_pos + 1;
            *condition = second_comma + 1;
        } else {
            *value = comma_pos + 1;
            *condition = NULL;
        }
        
        while (**key && isspace((unsigned char)**key)) (*key)++;
        end_ptr = *key + strlen(*key) - 1;
        while (end_ptr > *key && isspace((unsigned char)*end_ptr)) {
            *end_ptr = '\0';
            end_ptr--;
        }
        
        while (**value && isspace((unsigned char)**value)) (*value)++;
        end_ptr = *value + strlen(*value) - 1;
        while (end_ptr > *value && isspace((unsigned char)*end_ptr)) {
            *end_ptr = '\0';
            end_ptr--;
        }
        
        if (*condition) {
            while (**condition && isspace((unsigned char)**condition)) (*condition)++;
            end_ptr = *condition + strlen(*condition) - 1;
            while (end_ptr > *condition && isspace((unsigned char)*end_ptr)) {
                *end_ptr = '\0';
                end_ptr--;
            }
        }
    }
    else {
        *key = line_copy;
        *value = line_copy;
        
        while (**key && isspace((unsigned char)**key)) (*key)++;
        end_ptr = *key + strlen(*key) - 1;
        while (end_ptr > *key && isspace((unsigned char)*end_ptr)) {
            *end_ptr = '\0';
            end_ptr--;
        }
    }
}

char* get_string(const char* section, const char* key) {
    return get_string_default(section, key, NULL);
}

char* get_string_default(const char* section, const char* key, const char* default_value) {
    char far * far *lines;
    int line_count;
    int i;
    char* result;
    
    /* Use inf_section_find directly instead of get_section_lines */
    if (inf_section_find((const char far *)section, &lines, &line_count)) {
        for (i = 0; i < line_count; i++) {
            char *line_key;
            char *line_value;
            char *line_condition;
            
            parse_section_line((const char*)lines[i], &line_key, &line_value, &line_condition);
            
            if (line_key && strcmp(line_key, key) == 0) {
                result = NULL;
                if (line_value) {
                    result = (char*)malloc(strlen(line_value) + 1);
                    if (result) {
                        strcpy(result, line_value);
                    }
                }
                _ffree(lines);
                return result;
            }
        }
        _ffree(lines);
    }
    
    if (default_value) {
        result = (char*)malloc(strlen(default_value) + 1);
        if (result) {
            strcpy(result, default_value);
        }
        return result;
    }
    
    return NULL;
}

char* get_string_for_cpu(const char* base_section, const char* key, int cpu_type) {
    char section_name[100];
    char* result;
    
    if (cpu_type == CPU_386) {
        sprintf(section_name, "%s.win386", base_section);
        result = get_string(section_name, key);
        if (result) return result;
        
        sprintf(section_name, "%s.386", base_section);
        result = get_string(section_name, key);
        if (result) return result;
    } else if (cpu_type == CPU_286) {
        sprintf(section_name, "%s.286", base_section);
        result = get_string(section_name, key);
        if (result) return result;
    }
    
    return get_string(base_section, key);
}

int get_int(const char* section, const char* key, int default_value) {
    char* str_val;
    int result;
    
    str_val = get_string(section, key);
    if (str_val) {
        result = atoi(str_val);
        free(str_val);
        return result;
    }
    return default_value;
}

char* get_current_language(void) {
    char* result;
    
    result = (char*)malloc(strlen(current_language) + 1);
    if (result) {
        strcpy(result, current_language);
    }
    return result;
}


void free_resources(void) {
    inf_sections_free_all();
    machine_list_free();
    disk_list_free_all();
    mouse_list_free();
    kbdtype_list_free();
    kbdtable_list_free();
    language_list_free();
    network_list_free();  /* NEW: Free network list */
}


char* get_private_profile_string(const char* section, const char* key, 
                                const char* default_val, const char* filename) {
    return get_string_default(section, key, default_val);
}

int get_private_profile_int(const char* section, const char* key, 
                           int default_val, const char* filename) {
    char* str_val;
    int result;
    
    str_val = get_private_profile_string(section, key, NULL, filename);
    if (str_val) {
        result = atoi(str_val);
        free(str_val);
        return result;
    }
    return default_val;
}
