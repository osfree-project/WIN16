#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <malloc.h>
#include "log.h"
#include "section_list.h"

#define MAX_SECTION_LENGTH 50

typedef struct InfSection {
    char name[MAX_SECTION_LENGTH];
    char far * far *raw_lines;
    int raw_line_count;
    int raw_lines_allocated;
    struct InfSection far *next;
} InfSection;

static InfSection far *section_list_head = NULL;
#define MAX_SAFE_ITERATIONS 10000

static InfSection far *inf_section_create(const char far *name) {
    InfSection far *section;
    
//    log_message("inf_section_create: Creating section '%Fs'", name);
    
    section = (InfSection far *)_fmalloc(sizeof(InfSection));
    if (!section) {
        log_message("inf_section_create: ERROR - _fmalloc failed for section");
        return NULL;
    }
    
//    log_message("inf_section_create: Section allocated at %Fp", section);
    
    _fmemset(section, 0, sizeof(InfSection));
    _fstrncpy(section->name, name, MAX_SECTION_LENGTH - 1);
    section->name[MAX_SECTION_LENGTH - 1] = '\0';
    section->next = section_list_head;
    section_list_head = section;
    
//    log_message("inf_section_create: Section '%Fs' created successfully", name);
    return section;
}

static void inf_section_add_line(InfSection far *section, const char far *line) {
    char far *new_line;
    unsigned int len;
    
    if (!section || !line) {
        log_message("inf_section_add_line: ERROR - null section or line");
        return;
    }
    
    len = _fstrlen(line);
    if (len == 0) {
        log_message("inf_section_add_line: WARNING - empty line");
        return;
    }
    
//    log_message("inf_section_add_line: Adding line to section '%Fs': '%.50Fs'", section->name, line);
    
    new_line = (char far *)_fmalloc(len + 1);
    if (!new_line) {
        log_message("inf_section_add_line: ERROR - _fmalloc failed for line buffer");
        return;
    }
    
//    log_message("inf_section_add_line: Line buffer allocated at %Fp", new_line);
    _fstrcpy(new_line, line);
    
    if (!section->raw_lines) {
//        log_message("inf_section_add_line: Allocating initial raw_lines array");
        section->raw_lines_allocated = 16;
        section->raw_lines = (char far * far *)_fmalloc(section->raw_lines_allocated * sizeof(char far *));
        if (!section->raw_lines) {
            log_message("inf_section_add_line: ERROR - _fmalloc failed for raw_lines array");
            _ffree(new_line);
            return;
        }
        _fmemset(section->raw_lines, 0, section->raw_lines_allocated * sizeof(char far *));
//        log_message("inf_section_add_line: raw_lines array allocated successfully at %Fp", section->raw_lines);
    }
    
    if (section->raw_line_count >= section->raw_lines_allocated) {
        int new_size = section->raw_lines_allocated * 2;
        char far * far *new_array;
        
//        log_message("inf_section_add_line: Expanding raw_lines array from %d to %d", 
//                   section->raw_lines_allocated, new_size);
        
        new_array = (char far * far *)_fmalloc(new_size * sizeof(char far *));
        if (!new_array) {
            log_message("inf_section_add_line: ERROR - _fmalloc failed for expanded raw_lines array");
            _ffree(new_line);
            return;
        }
        
        _fmemset(new_array, 0, new_size * sizeof(char far *));
        _fmemcpy(new_array, section->raw_lines, section->raw_line_count * sizeof(char far *));
        
        _ffree(section->raw_lines);
        section->raw_lines = new_array;
        section->raw_lines_allocated = new_size;
        
//        log_message("inf_section_add_line: Array expanded successfully");
    }
    
    section->raw_lines[section->raw_line_count] = new_line;
    section->raw_line_count++;
//    log_message("inf_section_add_line: Line added successfully. Total lines in section: %d", section->raw_line_count);
}

int inf_section_find(const char far *section_name, char far * far * *lines, int *line_count) {
    InfSection far *current = section_list_head;
    int count = 0;
    
    if (!section_name || !lines || !line_count) {
        log_message("inf_section_find: ERROR - invalid parameters");
        return 0;
    }
    
    *lines = NULL;
    *line_count = 0;
    
//    log_message("inf_section_find: Searching for section '%Fs'", section_name);
    
    while (current != NULL) {
        count++;
        if (count > MAX_SAFE_ITERATIONS) {
            log_message("inf_section_find: ERROR - Possible infinite loop, breaking");
            break;
        }
        
//        log_message("inf_section_find: Checking section %d: '%Fs'", count, current->name);
        
        if (_fstrcmp(current->name, section_name) == 0) {
//            log_message("inf_section_find: Section '%Fs' found at %Fp", section_name, current);
            
            if (current->raw_line_count > 0) {
                *lines = (char far * far *)_fmalloc(current->raw_line_count * sizeof(char far *));
                if (!*lines) {
                    log_message("inf_section_find: ERROR - _fmalloc failed for lines array");
                    return 0;
                }
                
                _fmemcpy(*lines, current->raw_lines, current->raw_line_count * sizeof(char far *));
                *line_count = current->raw_line_count;
                
//                log_message("inf_section_find: Returning %d lines for section '%Fs'", *line_count, section_name);
                return 1;
            } else {
//                log_message("inf_section_find: Section '%Fs' has no lines", section_name);
                return 1;
            }
        }
        current = current->next;
    }
    
//    log_message("inf_section_find: Section '%Fs' not found", section_name);
    return 0;
}

/* NEW FUNCTION: Remove a specific section from the list */
int inf_section_remove(const char far *section_name) {
    InfSection far *current = section_list_head;
    InfSection far *prev = NULL;
    int i;
    
    if (!section_name) {
        log_message("inf_section_remove: ERROR - null section name");
        return 0;
    }
    
//    log_message("inf_section_remove: Removing section '%Fs'", section_name);
    
    while (current != NULL) {
        if (_fstrcmp(current->name, section_name) == 0) {
//            log_message("inf_section_remove: Found section to remove at %Fp", current);
            
            /* Free all lines in the section */
            if (current->raw_lines) {
//                log_message("inf_section_remove: Freeing %d lines", current->raw_line_count);
                for (i = 0; i < current->raw_line_count; i++) {
                    if (current->raw_lines[i]) {
                        _ffree(current->raw_lines[i]);
                    }
                }
                _ffree(current->raw_lines);
            }
            
            /* Unlink from list */
            if (prev) {
                prev->next = current->next;
            } else {
                section_list_head = current->next;
            }
            
            _ffree(current);
//            log_message("inf_section_remove: Section '%Fs' removed successfully", section_name);
            return 1;
        }
        prev = current;
        current = current->next;
    }
    
//    log_message("inf_section_remove: Section '%Fs' not found for removal", section_name);
    return 0;
}

void inf_sections_free_all(void) {
    InfSection far *current = section_list_head;
    InfSection far *next;
    int i;
    int section_count = 0;
    int total_lines_freed = 0;
    
//    log_message("inf_sections_free_all: Starting to free all sections");
    
    while (current != NULL) {
        section_count++;
        if (section_count > MAX_SAFE_ITERATIONS) {
            log_message("inf_sections_free_all: ERROR - Possible infinite loop, breaking");
            break;
        }
        
        next = current->next;
        
//        log_message("inf_sections_free_all: Freeing section %d: '%Fs'", section_count, current->name);
        
        if (current->raw_lines) {
//            log_message("inf_sections_free_all: Section has %d lines to free", current->raw_line_count);
            for (i = 0; i < current->raw_line_count; i++) {
                if (current->raw_lines[i]) {
//                    log_message("inf_sections_free_all: Freeing line %d at %Fp", i, current->raw_lines[i]);
                    _ffree(current->raw_lines[i]);
                    total_lines_freed++;
                }
            }
//            log_message("inf_sections_free_all: Freeing raw_lines array at %Fp", current->raw_lines);
            _ffree(current->raw_lines);
        } else {
//            log_message("inf_sections_free_all: Section has no raw_lines array");
        }
        
//        log_message("inf_sections_free_all: Freeing section structure at %Fp", current);
        _ffree(current);
        current = next;
    }
    
    section_list_head = NULL;
//    log_message("inf_sections_free_all: Completed. Freed %d sections and %d lines", section_count, total_lines_freed);
}

void load_inf_file(const char far *filename) {
    FILE *file;
    char line[256];
    char current_section[64] = "";
    InfSection far *current_section_ptr = NULL;
    int line_num = 0;
    char *end;
    unsigned int filename_len;
    char near_filename[260];
    int section_count = 0;
    int line_count = 0;
    char far *far_section_name;
    char far *far_line;
    char far * far *dummy_lines;
    int dummy_count;
    
//    log_message("load_inf_file: Starting INF file load");
    
    if (!filename) {
        log_message("load_inf_file: ERROR - null filename");
        return;
    }
    
    filename_len = _fstrlen(filename);
    if (filename_len >= sizeof(near_filename)) {
        filename_len = sizeof(near_filename) - 1;
        log_message("load_inf_file: WARNING - filename truncated from %u to %u", _fstrlen(filename), filename_len);
    }
    
    _fstrncpy(near_filename, filename, filename_len);
    near_filename[filename_len] = '\0';
    
//    log_message("load_inf_file: Loading INF file: '%s' (converted from far)", near_filename);
    
    inf_sections_free_all();
    
    file = fopen(near_filename, "rt");
    if (!file) {
        log_message("load_inf_file: ERROR - Cannot open INF file '%s'", near_filename);
        return;
    }
    
//    log_message("load_inf_file: INF file opened successfully");

    while (fgets(line, sizeof(line), file)) {
        line_num++;
        
        if (line_num > MAX_SAFE_ITERATIONS) {
            log_message("load_inf_file: WARNING - Too many lines in INF file, stopping at line %d", line_num);
            break;
        }
        
//        log_message("load_inf_file: Processing line %d: '%.50s'", line_num, line);
        
        line[strcspn(line, "\r\n")] = '\0';
        
        if (line[0] == '\0') {
//            log_message("load_inf_file: Line %d - empty line, skipping", line_num);
            continue;
        }
        
        if (line[0] == ';') {
//            log_message("load_inf_file: Line %d - comment, skipping", line_num);
            continue;
        }

        if (line[0] == '[') {
//            log_message("load_inf_file: Line %d - section header detected", line_num);
            end = strchr(line, ']');
            if (end) {
                *end = '\0';
                strncpy(current_section, line + 1, sizeof(current_section) - 1);
                current_section[sizeof(current_section) - 1] = '\0';
                section_count++;
                
//                log_message("load_inf_file: Found section '%s' (section #%d)", current_section, section_count);
                
                far_section_name = (char far *)_fmalloc(strlen(current_section) + 1);
                if (far_section_name) {
                    _fstrcpy(far_section_name, current_section);
                    
                    if (!inf_section_find(far_section_name, &dummy_lines, &dummy_count)) {
//                        log_message("load_inf_file: Creating new section '%s'", current_section);
                        current_section_ptr = inf_section_create(far_section_name);
                    } else {
                        _ffree(dummy_lines);
                    }
                    _ffree(far_section_name);
                }
            }
            continue;
        }

        if (current_section_ptr && current_section[0] != '\0') {
//            log_message("load_inf_file: Adding line to section '%s'", current_section);
            
            far_line = (char far *)_fmalloc(strlen(line) + 1);
            if (far_line) {
                _fstrcpy(far_line, line);
                inf_section_add_line(current_section_ptr, far_line);
                _ffree(far_line);
                line_count++;
            }
        }
    }
    
    if (ferror(file)) {
        log_message("load_inf_file: ERROR - file read error occurred");
    }
    
    fclose(file);
//    log_message("load_inf_file: INF file loading completed. Processed %d lines, %d sections, %d data lines", 
//                line_num, section_count, line_count);
}
