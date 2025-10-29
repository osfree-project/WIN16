#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <malloc.h>
#include "resources.h"
#include "log.h"
#include "inf_parser.h"
#include "file_list.h"
#include "section_list.h"
#include "disk_list.h"
#include "machine_list.h"
#include "display_list.h"
#include "mouse_list.h"
#include "kbdtype_list.h"
#include "kbdtable_list.h"
#include "language_list.h"
#include "network_list.h"

void remove_disk_prefix(char *filename) {
    char *colon_pos = strchr(filename, ':');
    if (colon_pos != NULL) {
        memmove(filename, colon_pos + 1, strlen(colon_pos + 1) + 1);
    }
}

/* Helper function to trim whitespace and quotes from a string */
static void trim_string(char *str) {
    char *start = str;
    char *end;
    
    if (!str || strlen(str) == 0) return;
    
    /* Trim leading spaces */
    while (*start && isspace((unsigned char)*start)) start++;
    
    /* Trim trailing spaces */
    end = start + strlen(start) - 1;
    while (end > start && isspace((unsigned char)*end)) {
        *end = '\0';
        end--;
    }
    
    /* Remove surrounding quotes if present */
    if (start[0] == '"' && end[0] == '"' && end > start) {
        memmove(start, start + 1, (end - start) - 1);
        start[(end - start) - 1] = '\0';
    }
    
    /* Move back to original if we trimmed leading spaces */
    if (start != str) {
        memmove(str, start, strlen(start) + 1);
    }
}

/* NEW: Function to check if a line is a new machine entry */
static int is_machine_start_line(const char *line) {
    const char *temp = line;
    const char *first_quote, *second_quote, *comma_pos, *third_quote, *fourth_quote;
    
    if (!line || line[0] != '"') return 0;
    
    /* Skip leading whitespace */
    while (*temp && isspace((unsigned char)*temp)) temp++;
    if (*temp != '"') return 0;
    
    first_quote = temp;
    second_quote = strchr(first_quote + 1, '"');
    if (!second_quote) return 0;
    
    /* Look for comma after second quote */
    comma_pos = second_quote + 1;
    while (*comma_pos && isspace((unsigned char)*comma_pos)) comma_pos++;
    if (*comma_pos != ',') return 0;
    
    /* Look for third quote (start of ID) */
    third_quote = comma_pos + 1;
    while (*third_quote && isspace((unsigned char)*third_quote)) third_quote++;
    if (*third_quote != '"') return 0;
    
    fourth_quote = strchr(third_quote + 1, '"');
    if (!fourth_quote) return 0;
    
    return 1;
}

/* FIXED: Improved machine entry parser with better line detection */
static int parse_machine_entry(char far * far *lines, int start_index, int line_count, MachineEntry* entry) {
    int i;
    char line_copy[512];
    char *first_quote, *second_quote, *third_quote, *fourth_quote;
    char *temp;
    int desc_len;
    int found_machine = 0;
    int id_len;
    char *comma_pos;
    char *comment_pos;
    char *end_ptr;
    char near_desc[256];
    char near_id[100];
    int field_index;
    unsigned int line_len;
    
    /* Initialize entry */
    memset(entry, 0, sizeof(MachineEntry));
    field_index = 0;
    
    for (i = start_index; i < line_count; i++) {
        if (!lines[i]) continue;
        
        /* Safely copy far string to near buffer */
        line_len = _fstrlen(lines[i]);
        if (line_len >= sizeof(line_copy)) {
            line_len = sizeof(line_copy) - 1;
        }
        _fstrncpy(line_copy, lines[i], line_len);
        line_copy[line_len] = '\0';
        
        /* Skip empty lines and comments */
        temp = line_copy;
        while (*temp && isspace((unsigned char)*temp)) temp++;
        if (*temp == '\0' || *temp == ';') continue;
        
        /* Check if this line starts a new machine entry */
        if (is_machine_start_line(temp)) {
            if (found_machine) {
                /* We found a new machine, return current index */
                return i;
            }
            
            /* Parse machine description and ID */
            first_quote = temp;
            second_quote = strchr(first_quote + 1, '"');
            
            if (second_quote != NULL) {
                /* Look for comma after second quote */
                comma_pos = second_quote + 1;
                while (*comma_pos && isspace((unsigned char)*comma_pos)) comma_pos++;
                
                if (*comma_pos == ',') {
                    /* Now look for third quote (start of ID) */
                    third_quote = comma_pos + 1;
                    while (*third_quote && isspace((unsigned char)*third_quote)) third_quote++;
                    
                    if (*third_quote == '"') {
                        fourth_quote = strchr(third_quote + 1, '"');
                        
                        if (fourth_quote != NULL) {
                            /* We have a valid machine description line */
                            desc_len = second_quote - (first_quote + 1);
                            if (desc_len > 0 && desc_len < (int)sizeof(near_desc)) {
                                strncpy(near_desc, first_quote + 1, desc_len);
                                near_desc[desc_len] = '\0';
                                trim_string(near_desc);
                                
                                /* Allocate far memory for description */
                                entry->description = (char far*)_fmalloc(strlen(near_desc) + 1);
                                if (entry->description) {
                                    _fstrcpy(entry->description, near_desc);
                                }
                            }
                            
                            /* Extract machine ID */
                            id_len = fourth_quote - (third_quote + 1);
                            if (id_len > 0 && id_len < (int)sizeof(near_id)) {
                                strncpy(near_id, third_quote + 1, id_len);
                                near_id[id_len] = '\0';
                                trim_string(near_id);
                                
                                /* Allocate far memory for machine ID */
                                entry->machine_id = (char far*)_fmalloc(strlen(near_id) + 1);
                                if (entry->machine_id) {
                                    _fstrcpy(entry->machine_id, near_id);
                                }
                            }
                            
                            log_message("parse_machine_entry: Found machine '%s' with ID '%s'", 
                                       near_desc, near_id);
                            
                            found_machine = 1;
                            field_index = 0; /* Start with first field */
                            continue; /* Move to next line for field data */
                        }
                    }
                }
            }
        } else if (found_machine) {
            /* This is a continuation line for current machine configuration */
            temp = line_copy;
            
            /* Skip empty lines and comments in continuation */
            while (*temp && isspace((unsigned char)*temp)) temp++;
            if (*temp == '\0' || *temp == ';') continue;
            
            /* Remove comments from continuation lines */
            comment_pos = strchr(temp, ';');
            if (comment_pos) *comment_pos = '\0';
            
            /* Re-trim after comment removal */
            temp = line_copy;
            while (*temp && isspace((unsigned char)*temp)) temp++;
            if (*temp == '\0') continue;
            
            /* Trim the line */
            end_ptr = temp + strlen(temp) - 1;
            while (end_ptr > temp && isspace((unsigned char)*end_ptr)) {
                *end_ptr = '\0';
                end_ptr--;
            }
            
            /* Skip if empty after trimming */
            if (strlen(temp) == 0) continue;
            
            /* Trim quotes if present */
            trim_string(temp);
            
            if (strlen(temp) > 0) {
                char far *far_value;
                unsigned int temp_len = strlen(temp);
                
                /* Allocate far memory for the value */
                far_value = (char far*)_fmalloc(temp_len + 1);
                if (!far_value) {
                    log_message("parse_machine_entry: Memory allocation failed for field %d", field_index);
                    continue;
                }
                _fstrcpy(far_value, temp);
                
                /* Direct assignment to MachineEntry fields based on field index */
                switch (field_index) {
                    case 0: /* system_drv */
                        if (strcmp(temp, "system") == 0) {
                            if (entry->system_drv) _ffree(entry->system_drv);
                            entry->system_drv = (char far*)_fmalloc(strlen("system.drv") + 1);
                            if (entry->system_drv) _fstrcpy(entry->system_drv, "system.drv");
                            _ffree(far_value);
                        } else {
                            entry->system_drv = far_value;
                        }
                        log_message("parse_machine_entry: system_drv = '%s'", temp);
                        break;
                    case 1: /* kbd_drv */
                        entry->kbd_drv = far_value;
                        log_message("parse_machine_entry: kbd_drv = '%s'", temp);
                        break;
                    case 2: /* kbd_type */
                        entry->kbd_type = far_value;
                        log_message("parse_machine_entry: kbd_type = '%s'", temp);
                        break;
                    case 3: /* mouse_drv */
                        entry->mouse_drv = far_value;
                        log_message("parse_machine_entry: mouse_drv = '%s'", temp);
                        break;
                    case 4: /* disp_drv */
                        entry->disp_drv = far_value;
                        log_message("parse_machine_entry: disp_drv = '%s'", temp);
                        break;
                    case 5: /* sound_drv */
                        entry->sound_drv = far_value;
                        log_message("parse_machine_entry: sound_drv = '%s'", temp);
                        break;
                    case 6: /* comm_drv */
                        entry->comm_drv = far_value;
                        log_message("parse_machine_entry: comm_drv = '%s'", temp);
                        break;
                    case 7: /* himem_switch */
                        entry->himem_switch = far_value;
                        log_message("parse_machine_entry: himem_switch = '%s'", temp);
                        break;
                    case 8: /* ebios */
                        entry->ebios = far_value;
                        log_message("parse_machine_entry: ebios = '%s'", temp);
                        break;
                    default: /* cookies */
                        if (entry->cookie_count < 10) {
                            entry->cookies[entry->cookie_count] = far_value;
                            log_message("parse_machine_entry: cookie[%d] = '%s'", entry->cookie_count, temp);
                            entry->cookie_count++;
                        } else {
                            _ffree(far_value); /* Free if we exceed cookie limit */
                        }
                        break;
                }
                field_index++;
            }
        }
    }
    
    return line_count; /* Reached end of section */
}

/* FIXED: Parse disks section with proper disk character extraction */
static void parse_disks_section(void) {
    char far * far *lines;
    int line_count;
    int i;
    char line_copy[256];
    char disk_char;
    char path[256];
    char label[256];
    char tag[256];
    char *equal_pos;
    char *comma1, *comma2;
    unsigned int line_len;
    char *temp;
    char *disk_end;
    
    log_message("parse_disks_section: Starting disks section parsing");
    
    if (!inf_section_find("disks", &lines, &line_count)) {
        log_message("parse_disks_section: No [disks] section found");
        return;
    }
    
    log_message("parse_disks_section: Found [disks] section with %d lines", line_count);
    
    for (i = 0; i < line_count; i++) {
        if (!lines[i]) continue;
        
        /* Safely copy the line for processing */
        line_len = _fstrlen(lines[i]);
        if (line_len >= sizeof(line_copy)) {
            line_len = sizeof(line_copy) - 1;
        }
        _fstrncpy(line_copy, lines[i], line_len);
        line_copy[line_len] = '\0';
        
        log_message("parse_disks_section: Processing line: '%s'", line_copy);
        
        /* Skip empty lines and comments */
        temp = line_copy;
        while (*temp && isspace((unsigned char)*temp)) temp++;
        if (*temp == '\0' || *temp == ';') continue;
        
        /* Find the equal sign separating disk number from the rest */
        equal_pos = strchr(temp, '=');
        if (!equal_pos) {
            log_message("parse_disks_section: WARNING - no '=' found in line: %s", line_copy);
            continue;
        }
        
        /* Extract disk character (before '=') - get first non-space character */
        *equal_pos = '\0';
        
        /* Find the last non-space character before '=' */
        disk_end = equal_pos - 1;
        while (disk_end > temp && isspace((unsigned char)*disk_end)) {
            disk_end--;
        }
        
        if (disk_end >= temp) {
            disk_char = *disk_end;
        } else {
            log_message("parse_disks_section: WARNING - cannot extract disk character from: %s", line_copy);
            continue;
        }
        
        /* Validate disk character */
        if (disk_char < '1' || disk_char > '9') {
            log_message("parse_disks_section: WARNING - invalid disk character: '%c'", disk_char);
            continue;
        }
        
        /* Skip whitespace after '=' */
        equal_pos++;
        while (*equal_pos && isspace((unsigned char)*equal_pos)) equal_pos++;
        
        /* Now parse the three comma-separated values: path, label, tag */
        comma1 = strchr(equal_pos, ',');
        if (!comma1) {
            log_message("parse_disks_section: WARNING - no comma found in line: %s", line_copy);
            continue;
        }
        
        *comma1 = '\0';
        strncpy(path, equal_pos, sizeof(path) - 1);
        path[sizeof(path) - 1] = '\0';
        trim_string(path);
        
        comma1++;
        while (*comma1 && isspace((unsigned char)*comma1)) comma1++;
        
        comma2 = strchr(comma1, ',');
        if (!comma2) {
            log_message("parse_disks_section: WARNING - only one comma found in line: %s", line_copy);
            continue;
        }
        
        *comma2 = '\0';
        strncpy(label, comma1, sizeof(label) - 1);
        label[sizeof(label) - 1] = '\0';
        trim_string(label);
        
        comma2++;
        while (*comma2 && isspace((unsigned char)*comma2)) comma2++;
        strncpy(tag, comma2, sizeof(tag) - 1);
        tag[sizeof(tag) - 1] = '\0';
        trim_string(tag);
        
        log_message("parse_disks_section: Parsed disk '%c': path='%s', label='%s', tag='%s'", 
                   disk_char, path, label, tag);
        
        /* Add to disk list */
        disk_list_add(disk_char, path, label, tag);
    }
    
    _ffree(lines);
    
    /* Remove disks section after parsing to free memory */
    inf_section_remove("disks");
    log_message("parse_disks_section: Disks section removed after processing");
    
    log_message("parse_disks_section: Disks section parsing completed");
}

/* FIXED: Parse machine section with proper continuation line handling */
void parse_machine_section(void) {
    char far * far *lines;
    int line_count;
    int i;
    int j;
    MachineEntry temp_entry;
    int current_index = 0;
    int machine_count = 0;
    int next_index;
    char desc_buffer[256];
    char value_buffer[256];
    
    log_message("parse_machine_section: Starting machine section parsing");
    
    /* Initialize machine list */
    machine_list_init();
    
    /* Use inf_section_find to get machine section */
    if (!inf_section_find("machine", &lines, &line_count)) {
        log_message("parse_machine_section: No [machine] section found");
        return;
    }
    
    log_message("parse_machine_section: Found [machine] section with %d lines", line_count);
    
    /* Parse machine entries */
    current_index = 0;
    while (current_index < line_count) {
        /* Initialize temp_entry for each machine */
        memset(&temp_entry, 0, sizeof(MachineEntry));
        
        next_index = parse_machine_entry(lines, current_index, line_count, &temp_entry);
        
        if (temp_entry.description != NULL) {
            /* Add to machine list */
            machine_list_add(&temp_entry);
            machine_count++;
            
            /* Convert far string to near for logging */
            if (temp_entry.description) {
                unsigned int desc_len = _fstrlen(temp_entry.description);
                if (desc_len >= sizeof(desc_buffer)) {
                    desc_len = sizeof(desc_buffer) - 1;
                }
                _fstrncpy(desc_buffer, temp_entry.description, desc_len);
                desc_buffer[desc_len] = '\0';
            } else {
                strcpy(desc_buffer, "NULL");
            }
            
            log_message("parse_machine_section: Successfully added machine %d: '%s'", 
                       machine_count, desc_buffer);
        } else {
            log_message("parse_machine_section: No machine found at index %d", current_index);
            /* If no machine found and no progress, break to avoid infinite loop */
            if (next_index <= current_index) {
                break;
            }
        }
        
        if (next_index <= current_index) {
            /* No progress made, break to avoid infinite loop */
            log_message("parse_machine_section: No progress, breaking loop at index %d", current_index);
            break;
        }
        current_index = next_index;
    }
    
    _ffree(lines);
    
    /* Remove machine section after parsing to free memory */
    inf_section_remove("machine");
    log_message("parse_machine_section: Machine section removed after processing");
    
    /* Log parsing results */
    log_message("parse_machine_section: Machine section parsing completed, found %d entries", 
               machine_list_count());
    
    /* Log full details of each machine entry */
    for (i = 0; i < machine_list_count(); i++) {
        MachineEntry* entry = machine_list_get(i);
        if (entry) {
            /* Convert far strings to near for safe logging */
            if (entry->description) {
                unsigned int desc_len = _fstrlen(entry->description);
                if (desc_len >= sizeof(desc_buffer)) {
                    desc_len = sizeof(desc_buffer) - 1;
                }
                _fstrncpy(desc_buffer, entry->description, desc_len);
                desc_buffer[desc_len] = '\0';
            } else {
                strcpy(desc_buffer, "NULL");
            }
            
            log_message("  Machine %d: '%s'", i, desc_buffer);
            
            /* Log all fields */
            if (entry->machine_id) {
                unsigned int len = _fstrlen(entry->machine_id);
                if (len >= sizeof(value_buffer)) len = sizeof(value_buffer) - 1;
                _fstrncpy(value_buffer, entry->machine_id, len);
                value_buffer[len] = '\0';
                log_message("    machine_id: '%s'", value_buffer);
            } else {
                log_message("    machine_id: NULL");
            }
            
            if (entry->system_drv) {
                unsigned int len = _fstrlen(entry->system_drv);
                if (len >= sizeof(value_buffer)) len = sizeof(value_buffer) - 1;
                _fstrncpy(value_buffer, entry->system_drv, len);
                value_buffer[len] = '\0';
                log_message("    system_drv: '%s'", value_buffer);
            } else {
                log_message("    system_drv: NULL");
            }
            
            if (entry->kbd_drv) {
                unsigned int len = _fstrlen(entry->kbd_drv);
                if (len >= sizeof(value_buffer)) len = sizeof(value_buffer) - 1;
                _fstrncpy(value_buffer, entry->kbd_drv, len);
                value_buffer[len] = '\0';
                log_message("    kbd_drv: '%s'", value_buffer);
            } else {
                log_message("    kbd_drv: NULL");
            }
            
            if (entry->kbd_type) {
                unsigned int len = _fstrlen(entry->kbd_type);
                if (len >= sizeof(value_buffer)) len = sizeof(value_buffer) - 1;
                _fstrncpy(value_buffer, entry->kbd_type, len);
                value_buffer[len] = '\0';
                log_message("    kbd_type: '%s'", value_buffer);
            } else {
                log_message("    kbd_type: NULL");
            }
            
            if (entry->mouse_drv) {
                unsigned int len = _fstrlen(entry->mouse_drv);
                if (len >= sizeof(value_buffer)) len = sizeof(value_buffer) - 1;
                _fstrncpy(value_buffer, entry->mouse_drv, len);
                value_buffer[len] = '\0';
                log_message("    mouse_drv: '%s'", value_buffer);
            } else {
                log_message("    mouse_drv: NULL");
            }
            
            if (entry->disp_drv) {
                unsigned int len = _fstrlen(entry->disp_drv);
                if (len >= sizeof(value_buffer)) len = sizeof(value_buffer) - 1;
                _fstrncpy(value_buffer, entry->disp_drv, len);
                value_buffer[len] = '\0';
                log_message("    disp_drv: '%s'", value_buffer);
            } else {
                log_message("    disp_drv: NULL");
            }
            
            if (entry->sound_drv) {
                unsigned int len = _fstrlen(entry->sound_drv);
                if (len >= sizeof(value_buffer)) len = sizeof(value_buffer) - 1;
                _fstrncpy(value_buffer, entry->sound_drv, len);
                value_buffer[len] = '\0';
                log_message("    sound_drv: '%s'", value_buffer);
            } else {
                log_message("    sound_drv: NULL");
            }
            
            if (entry->comm_drv) {
                unsigned int len = _fstrlen(entry->comm_drv);
                if (len >= sizeof(value_buffer)) len = sizeof(value_buffer) - 1;
                _fstrncpy(value_buffer, entry->comm_drv, len);
                value_buffer[len] = '\0';
                log_message("    comm_drv: '%s'", value_buffer);
            } else {
                log_message("    comm_drv: NULL");
            }
            
            if (entry->himem_switch) {
                unsigned int len = _fstrlen(entry->himem_switch);
                if (len >= sizeof(value_buffer)) len = sizeof(value_buffer) - 1;
                _fstrncpy(value_buffer, entry->himem_switch, len);
                value_buffer[len] = '\0';
                log_message("    himem_switch: '%s'", value_buffer);
            } else {
                log_message("    himem_switch: NULL");
            }
            
            if (entry->ebios) {
                unsigned int len = _fstrlen(entry->ebios);
                if (len >= sizeof(value_buffer)) len = sizeof(value_buffer) - 1;
                _fstrncpy(value_buffer, entry->ebios, len);
                value_buffer[len] = '\0';
                log_message("    ebios: '%s'", value_buffer);
            } else {
                log_message("    ebios: NULL");
            }
            
            /* Cookies */
            log_message("    cookies: %d entries", entry->cookie_count);
            for (j = 0; j < entry->cookie_count; j++) {
                if (entry->cookies[j]) {
                    unsigned int len = _fstrlen(entry->cookies[j]);
                    if (len >= sizeof(value_buffer)) len = sizeof(value_buffer) - 1;
                    _fstrncpy(value_buffer, entry->cookies[j], len);
                    value_buffer[len] = '\0';
                    log_message("      [%d]: '%s'", j, value_buffer);
                } else {
                    log_message("      [%d]: NULL", j);
                }
            }
        } else {
            log_message("  Machine %d: NULL entry", i);
        }
    }
}

/* NEW: Improved display section parser with proper field handling */
static int parse_display_fields(char *line, char *fields[], int max_fields) {
    int field_count = 0;
    char *ptr = line;
    int in_quotes = 0;
    char *field_start = ptr;
    
    while (*ptr && field_count < max_fields) {
        if (*ptr == '"') {
            in_quotes = !in_quotes;
        } else if (*ptr == ',' && !in_quotes) {
            /* End of field */
            *ptr = '\0';
            fields[field_count++] = field_start;
            field_start = ptr + 1;
            
            /* Skip whitespace after comma */
            while (*field_start && isspace((unsigned char)*field_start)) {
                field_start++;
            }
        }
        ptr++;
    }
    
    /* Add the last field */
    if (field_count < max_fields && *field_start) {
        fields[field_count++] = field_start;
    }
    
    return field_count;
}

/* FIXED: Parse display section with proper field extraction and far pointers */
void parse_display_section(void) {
    char far * far *lines;
    int line_count;
    int i;
    char line_copy[512];
    char *equal_pos;
    char *temp;
    char display_name[100];
    char *fields[9]; /* driver_file, description, resolution, grabber_286, logo_code, vdd_file, grabber_386, ega_sys, logo_data */
    int field_count;
    unsigned int line_len;
    char driver_file[100];
    char description[256];
    char resolution[100];
    char grabber_286[100];
    char logo_code[100];
    char vdd_file[100];
    char grabber_386[100];
    char ega_sys[100];
    char logo_data[100];
    
    log_message("parse_display_section: Starting display section parsing");
    
    if (!inf_section_find("display", &lines, &line_count)) {
        log_message("parse_display_section: No [display] section found");
        return;
    }
    
    log_message("parse_display_section: Found [display] section with %d lines", line_count);
    
    /* Initialize display list */
    display_list_init();
    
    for (i = 0; i < line_count; i++) {
        if (!lines[i]) continue;
        
        /* Safely copy far string to near buffer */
        line_len = _fstrlen(lines[i]);
        if (line_len >= sizeof(line_copy)) {
            line_len = sizeof(line_copy) - 1;
        }
        _fstrncpy(line_copy, lines[i], line_len);
        line_copy[line_len] = '\0';
        
        log_message("parse_display_section: Processing line: '%s'", line_copy);
        
        /* Skip empty lines and comments */
        temp = line_copy;
        while (*temp && isspace((unsigned char)*temp)) temp++;
        if (*temp == '\0' || *temp == ';') continue;
        
        /* Find equal sign */
        equal_pos = strchr(temp, '=');
        if (!equal_pos) {
            log_message("parse_display_section: WARNING - no '=' found in line: %s", line_copy);
            continue;
        }
        
        /* Extract display name (before '=') */
        *equal_pos = '\0';
        strncpy(display_name, temp, sizeof(display_name) - 1);
        display_name[sizeof(display_name) - 1] = '\0';
        trim_string(display_name);
        
        /* Parse fields after '=' */
        temp = equal_pos + 1;
        while (*temp && isspace((unsigned char)*temp)) temp++;
        
        /* Initialize fields */
        memset(fields, 0, sizeof(fields));
        field_count = parse_display_fields(temp, fields, 9);
        
        if (field_count < 9) {
            log_message("parse_display_section: WARNING - expected 9 fields, got %d in line: %s", field_count, line_copy);
            continue;
        }
        
        /* Process each field */
        driver_file[0] = '\0';
        description[0] = '\0';
        resolution[0] = '\0';
        grabber_286[0] = '\0';
        logo_code[0] = '\0';
        vdd_file[0] = '\0';
        grabber_386[0] = '\0';
        ega_sys[0] = '\0';
        logo_data[0] = '\0';
        
        /* Copy and trim each field */
        if (fields[0]) {
            strncpy(driver_file, fields[0], sizeof(driver_file) - 1);
            driver_file[sizeof(driver_file) - 1] = '\0';
            trim_string(driver_file);
        }
        
        if (fields[1]) {
            strncpy(description, fields[1], sizeof(description) - 1);
            description[sizeof(description) - 1] = '\0';
            trim_string(description);
        }
        
        if (fields[2]) {
            strncpy(resolution, fields[2], sizeof(resolution) - 1);
            resolution[sizeof(resolution) - 1] = '\0';
            trim_string(resolution);
        }
        
        if (fields[3]) {
            strncpy(grabber_286, fields[3], sizeof(grabber_286) - 1);
            grabber_286[sizeof(grabber_286) - 1] = '\0';
            trim_string(grabber_286);
        }
        
        if (fields[4]) {
            strncpy(logo_code, fields[4], sizeof(logo_code) - 1);
            logo_code[sizeof(logo_code) - 1] = '\0';
            trim_string(logo_code);
        }
        
        if (fields[5]) {
            strncpy(vdd_file, fields[5], sizeof(vdd_file) - 1);
            vdd_file[sizeof(vdd_file) - 1] = '\0';
            trim_string(vdd_file);
        }
        
        if (fields[6]) {
            strncpy(grabber_386, fields[6], sizeof(grabber_386) - 1);
            grabber_386[sizeof(grabber_386) - 1] = '\0';
            trim_string(grabber_386);
        }
        
        if (fields[7]) {
            strncpy(ega_sys, fields[7], sizeof(ega_sys) - 1);
            ega_sys[sizeof(ega_sys) - 1] = '\0';
            trim_string(ega_sys);
        }
        
        if (fields[8]) {
            strncpy(logo_data, fields[8], sizeof(logo_data) - 1);
            logo_data[sizeof(logo_data) - 1] = '\0';
            trim_string(logo_data);
        }
        
        log_message("parse_display_section: Parsed display '%s':", display_name);
        log_message("  driver_file: '%s'", driver_file);
        log_message("  description: '%s'", description);
        log_message("  resolution: '%s'", resolution);
        log_message("  grabber_286: '%s'", grabber_286);
        log_message("  logo_code: '%s'", logo_code);
        log_message("  vdd_file: '%s'", vdd_file);
        log_message("  grabber_386: '%s'", grabber_386);
        log_message("  ega_sys: '%s'", ega_sys);
        log_message("  logo_data: '%s'", logo_data);
        
        /* Add to display list - now uses far pointers internally */
        display_list_add(display_name, driver_file, description, resolution,
                       grabber_286, logo_code, vdd_file, grabber_386,
                       ega_sys, logo_data);
    }
    
    _ffree(lines);
    
    /* Remove display section after parsing to free memory */
    inf_section_remove("display");
    log_message("parse_display_section: Display section removed after processing");
    
    log_message("parse_display_section: Display section parsing completed");
}

/* NEW: Parse mouse section with proper field extraction */
void parse_mouse_section(void) {
    char far * far *lines;
    int line_count;
    int i;
    char line_copy[512];
    char *equal_pos;
    char *temp;
    char mouse_name[100];
    char *fields[4]; /* driver_file, description, vmd, dos_driver */
    int field_count;
    unsigned int line_len;
    char driver_file[100];
    char description[256];
    char vmd[100];
    char dos_driver[100];
    
    log_message("parse_mouse_section: Starting mouse section parsing");
    
    if (!inf_section_find("pointing.device", &lines, &line_count)) {
        log_message("parse_mouse_section: No [pointing.device] section found");
        return;
    }
    
    log_message("parse_mouse_section: Found [pointing.device] section with %d lines", line_count);
    
    /* Initialize mouse list */
    mouse_list_init();
    
    for (i = 0; i < line_count; i++) {
        if (!lines[i]) continue;
        
        /* Safely copy far string to near buffer */
        line_len = _fstrlen(lines[i]);
        if (line_len >= sizeof(line_copy)) {
            line_len = sizeof(line_copy) - 1;
        }
        _fstrncpy(line_copy, lines[i], line_len);
        line_copy[line_len] = '\0';
        
        log_message("parse_mouse_section: Processing line: '%s'", line_copy);
        
        /* Skip empty lines and comments */
        temp = line_copy;
        while (*temp && isspace((unsigned char)*temp)) temp++;
        if (*temp == '\0' || *temp == ';') continue;
        
        /* Find equal sign */
        equal_pos = strchr(temp, '=');
        if (!equal_pos) {
            log_message("parse_mouse_section: WARNING - no '=' found in line: %s", line_copy);
            continue;
        }
        
        /* Extract mouse name (before '=') */
        *equal_pos = '\0';
        strncpy(mouse_name, temp, sizeof(mouse_name) - 1);
        mouse_name[sizeof(mouse_name) - 1] = '\0';
        trim_string(mouse_name);
        
        /* Parse fields after '=' */
        temp = equal_pos + 1;
        while (*temp && isspace((unsigned char)*temp)) temp++;
        
        /* Initialize fields */
        memset(fields, 0, sizeof(fields));
        field_count = parse_display_fields(temp, fields, 4);
        
        if (field_count < 2) {
            log_message("parse_mouse_section: WARNING - expected at least 2 fields, got %d in line: %s", field_count, line_copy);
            continue;
        }
        
        /* Process each field */
        driver_file[0] = '\0';
        description[0] = '\0';
        vmd[0] = '\0';
        dos_driver[0] = '\0';
        
        /* Copy and trim each field */
        if (fields[0]) {
            strncpy(driver_file, fields[0], sizeof(driver_file) - 1);
            driver_file[sizeof(driver_file) - 1] = '\0';
            trim_string(driver_file);
        }
        
        if (fields[1]) {
            strncpy(description, fields[1], sizeof(description) - 1);
            description[sizeof(description) - 1] = '\0';
            trim_string(description);
        }
        
        if (fields[2]) {
            strncpy(vmd, fields[2], sizeof(vmd) - 1);
            vmd[sizeof(vmd) - 1] = '\0';
            trim_string(vmd);
        }
        
        if (fields[3]) {
            strncpy(dos_driver, fields[3], sizeof(dos_driver) - 1);
            dos_driver[sizeof(dos_driver) - 1] = '\0';
            trim_string(dos_driver);
        }
        
        log_message("parse_mouse_section: Parsed mouse '%s':", mouse_name);
        log_message("  driver_file: '%s'", driver_file);
        log_message("  description: '%s'", description);
        log_message("  vmd: '%s'", vmd);
        log_message("  dos_driver: '%s'", dos_driver);
        
        /* Add to mouse list */
        mouse_list_add(mouse_name, driver_file, description, vmd, dos_driver);
    }
    
    _ffree(lines);
    
    /* Remove mouse section after parsing to free memory */
    inf_section_remove("pointing.device");
    log_message("parse_mouse_section: Mouse section removed after processing");
    
    log_message("parse_mouse_section: Mouse section parsing completed, found %d entries", mouse_list_count());
}

/* NEW: Parse keyboard type section with proper field extraction */
void parse_kbdtype_section(void) {
    char far * far *lines;
    int line_count;
    int i;
    char line_copy[512];
    char *equal_pos;
    char *temp;
    char kbdtype_name[100];
    char *fields[2]; /* description, dll */
    int field_count;
    unsigned int line_len;
    char description[256];
    char dll[100];
    
    log_message("parse_kbdtype_section: Starting keyboard type section parsing");
    
    if (!inf_section_find("keyboard.types", &lines, &line_count)) {
        log_message("parse_kbdtype_section: No [keyboard.types] section found");
        return;
    }
    
    log_message("parse_kbdtype_section: Found [keyboard.types] section with %d lines", line_count);
    
    /* Initialize keyboard type list */
    kbdtype_list_init();
    
    for (i = 0; i < line_count; i++) {
        if (!lines[i]) continue;
        
        /* Safely copy far string to near buffer */
        line_len = _fstrlen(lines[i]);
        if (line_len >= sizeof(line_copy)) {
            line_len = sizeof(line_copy) - 1;
        }
        _fstrncpy(line_copy, lines[i], line_len);
        line_copy[line_len] = '\0';
        
        log_message("parse_kbdtype_section: Processing line: '%s'", line_copy);
        
        /* Skip empty lines and comments */
        temp = line_copy;
        while (*temp && isspace((unsigned char)*temp)) temp++;
        if (*temp == '\0' || *temp == ';') continue;
        
        /* Find equal sign */
        equal_pos = strchr(temp, '=');
        if (!equal_pos) {
            log_message("parse_kbdtype_section: WARNING - no '=' found in line: %s", line_copy);
            continue;
        }
        
        /* Extract keyboard type name (before '=') */
        *equal_pos = '\0';
        strncpy(kbdtype_name, temp, sizeof(kbdtype_name) - 1);
        kbdtype_name[sizeof(kbdtype_name) - 1] = '\0';
        trim_string(kbdtype_name);
        
        /* Parse fields after '=' */
        temp = equal_pos + 1;
        while (*temp && isspace((unsigned char)*temp)) temp++;
        
        /* Initialize fields */
        memset(fields, 0, sizeof(fields));
        field_count = parse_display_fields(temp, fields, 2);
        
        if (field_count < 1) {
            log_message("parse_kbdtype_section: WARNING - expected at least 1 field, got %d in line: %s", field_count, line_copy);
            continue;
        }
        
        /* Process each field */
        description[0] = '\0';
        dll[0] = '\0';
        
        /* Copy and trim each field */
        if (fields[0]) {
            strncpy(description, fields[0], sizeof(description) - 1);
            description[sizeof(description) - 1] = '\0';
            trim_string(description);
        }
        
        if (fields[1]) {
            strncpy(dll, fields[1], sizeof(dll) - 1);
            dll[sizeof(dll) - 1] = '\0';
            trim_string(dll);
        }
        
        log_message("parse_kbdtype_section: Parsed keyboard type '%s':", kbdtype_name);
        log_message("  description: '%s'", description);
        log_message("  dll: '%s'", dll);
        
        /* Add to keyboard type list */
        kbdtype_list_add(kbdtype_name, description, dll);
    }
    
    _ffree(lines);
    
    /* Remove keyboard type section after parsing to free memory */
    inf_section_remove("keyboard.types");
    log_message("parse_kbdtype_section: Keyboard type section removed after processing");
    
    log_message("parse_kbdtype_section: Keyboard type section parsing completed, found %d entries", kbdtype_list_count());
}

/* NEW: Parse keyboard table section with proper field extraction */
void parse_kbdlay_section(void) {
    char far * far *lines;
    int line_count;
    int i;
    char line_copy[512];
    char *equal_pos;
    char *temp;
    char kbdtable_name[100];
    char *fields[2]; /* dll, description */
    int field_count;
    unsigned int line_len;
    char dll[100];
    char description[256];
    
    log_message("parse_kbdlay_section: Starting keyboard table section parsing");
    
    if (!inf_section_find("keyboard.tables", &lines, &line_count)) {
        log_message("parse_kbdlay_section: No [keyboard.tables] section found");
        return;
    }
    
    log_message("parse_kbdlay_section: Found [keyboard.tables] section with %d lines", line_count);
    
    /* Initialize keyboard table list */
    kbdtable_list_init();
    
    for (i = 0; i < line_count; i++) {
        if (!lines[i]) continue;
        
        /* Safely copy far string to near buffer */
        line_len = _fstrlen(lines[i]);
        if (line_len >= sizeof(line_copy)) {
            line_len = sizeof(line_copy) - 1;
        }
        _fstrncpy(line_copy, lines[i], line_len);
        line_copy[line_len] = '\0';
        
        log_message("parse_kbdlay_section: Processing line: '%s'", line_copy);
        
        /* Skip empty lines and comments */
        temp = line_copy;
        while (*temp && isspace((unsigned char)*temp)) temp++;
        if (*temp == '\0' || *temp == ';') continue;
        
        /* Find equal sign */
        equal_pos = strchr(temp, '=');
        if (!equal_pos) {
            log_message("parse_kbdlay_section: WARNING - no '=' found in line: %s", line_copy);
            continue;
        }
        
        /* Extract keyboard table name (before '=') */
        *equal_pos = '\0';
        strncpy(kbdtable_name, temp, sizeof(kbdtable_name) - 1);
        kbdtable_name[sizeof(kbdtable_name) - 1] = '\0';
        trim_string(kbdtable_name);
        
        /* Parse fields after '=' */
        temp = equal_pos + 1;
        while (*temp && isspace((unsigned char)*temp)) temp++;
        
        /* Initialize fields */
        memset(fields, 0, sizeof(fields));
        field_count = parse_display_fields(temp, fields, 2);
        
        if (field_count < 1) {
            log_message("parse_kbdlay_section: WARNING - expected at least 1 field, got %d in line: %s", field_count, line_copy);
            continue;
        }
        
        /* Process each field */
        dll[0] = '\0';
        description[0] = '\0';
        
        /* Copy and trim each field */
        if (fields[0]) {
            strncpy(dll, fields[0], sizeof(dll) - 1);
            dll[sizeof(dll) - 1] = '\0';
            trim_string(dll);
        }
        
        if (fields[1]) {
            strncpy(description, fields[1], sizeof(description) - 1);
            description[sizeof(description) - 1] = '\0';
            trim_string(description);
        }
        
        log_message("parse_kbdlay_section: Parsed keyboard table '%s':", kbdtable_name);
        log_message("  dll: '%s'", dll);
        log_message("  description: '%s'", description);
        
        /* Add to keyboard table list */
        kbdtable_list_add(kbdtable_name, dll, description);
    }
    
    _ffree(lines);
    
    /* Remove keyboard table section after parsing to free memory */
    inf_section_remove("keyboard.tables");
    log_message("parse_kbdlay_section: Keyboard table section removed after processing");
    
    log_message("parse_kbdlay_section: Keyboard table section parsing completed, found %d entries", kbdtable_list_count());
}

/* NEW: Parse language section with proper field extraction */
void parse_language_section(void) {
    char far * far *lines;
    int line_count;
    int i;
    char line_copy[512];
    char *equal_pos;
    char *temp;
    char language_name[100];
    char *fields[2]; /* dll, description */
    int field_count;
    unsigned int line_len;
    char dll[100];
    char description[256];
    
    log_message("parse_language_section: Starting language section parsing");
    
    if (!inf_section_find("language", &lines, &line_count)) {
        log_message("parse_language_section: No [language] section found");
        return;
    }
    
    log_message("parse_language_section: Found [language] section with %d lines", line_count);
    
    /* Initialize language list */
    language_list_init();
    
    for (i = 0; i < line_count; i++) {
        if (!lines[i]) continue;
        
        /* Safely copy far string to near buffer */
        line_len = _fstrlen(lines[i]);
        if (line_len >= sizeof(line_copy)) {
            line_len = sizeof(line_copy) - 1;
        }
        _fstrncpy(line_copy, lines[i], line_len);
        line_copy[line_len] = '\0';
        
        log_message("parse_language_section: Processing line: '%s'", line_copy);
        
        /* Skip empty lines and comments */
        temp = line_copy;
        while (*temp && isspace((unsigned char)*temp)) temp++;
        if (*temp == '\0' || *temp == ';') continue;
        
        /* Find equal sign */
        equal_pos = strchr(temp, '=');
        if (!equal_pos) {
            log_message("parse_language_section: WARNING - no '=' found in line: %s", line_copy);
            continue;
        }
        
        /* Extract language name (before '=') */
        *equal_pos = '\0';
        strncpy(language_name, temp, sizeof(language_name) - 1);
        language_name[sizeof(language_name) - 1] = '\0';
        trim_string(language_name);
        
        /* Parse fields after '=' */
        temp = equal_pos + 1;
        while (*temp && isspace((unsigned char)*temp)) temp++;
        
        /* Initialize fields */
        memset(fields, 0, sizeof(fields));
        field_count = parse_display_fields(temp, fields, 2);
        
        if (field_count < 1) {
            log_message("parse_language_section: WARNING - expected at least 1 field, got %d in line: %s", field_count, line_copy);
            continue;
        }
        
        /* Process each field */
        dll[0] = '\0';
        description[0] = '\0';
        
        /* Copy and trim each field */
        if (fields[0]) {
            strncpy(dll, fields[0], sizeof(dll) - 1);
            dll[sizeof(dll) - 1] = '\0';
            trim_string(dll);
        }
        
        if (fields[1]) {
            strncpy(description, fields[1], sizeof(description) - 1);
            description[sizeof(description) - 1] = '\0';
            trim_string(description);
        }
        
        log_message("parse_language_section: Parsed language '%s':", language_name);
        log_message("  dll: '%s'", dll);
        log_message("  description: '%s'", description);
        
        /* Add to language list */
        language_list_add(language_name, dll, description);
    }
    
    _ffree(lines);
    
    /* Remove language section after parsing to free memory */
    inf_section_remove("language");
    log_message("parse_language_section: Language section removed after processing");
    
    log_message("parse_language_section: Language section parsing completed, found %d entries", language_list_count());
}

/* NEW: Parse network section with proper field extraction and variable VDD count */
void parse_network_section(void) {
    char far * far *lines;
    int line_count;
    int i;
    char line_copy[512];
    char *equal_pos;
    char *temp;
    char network_name[100];
    char *fields[20]; /* Increased field count to accommodate variable VDDs */
    int field_count;
    unsigned int line_len;
    char driver_file[100];
    char description[256];
    char help_file[100];
    char opt_file[100];
    char winini_sect_name[100];
    char sysini_sect_name[100];
    char *vdd_array[MAX_VDD_COUNT]; /* Array for VDD drivers */
    int vdd_count = 0;
    int j;
    
    log_message("parse_network_section: Starting network section parsing");
    
    if (!inf_section_find("network", &lines, &line_count)) {
        log_message("parse_network_section: No [network] section found");
        return;
    }
    
    log_message("parse_network_section: Found [network] section with %d lines", line_count);
    
    /* Initialize network list */
    network_list_init();
    
    for (i = 0; i < line_count; i++) {
        if (!lines[i]) continue;
        
        /* Safely copy far string to near buffer */
        line_len = _fstrlen(lines[i]);
        if (line_len >= sizeof(line_copy)) {
            line_len = sizeof(line_copy) - 1;
        }
        _fstrncpy(line_copy, lines[i], line_len);
        line_copy[line_len] = '\0';
        
        log_message("parse_network_section: Processing line: '%s'", line_copy);
        
        /* Skip empty lines and comments */
        temp = line_copy;
        while (*temp && isspace((unsigned char)*temp)) temp++;
        if (*temp == '\0' || *temp == ';') continue;
        
        /* Find equal sign */
        equal_pos = strchr(temp, '=');
        if (!equal_pos) {
            log_message("parse_network_section: WARNING - no '=' found in line: %s", line_copy);
            continue;
        }
        
        /* Extract network name (before '=') */
        *equal_pos = '\0';
        strncpy(network_name, temp, sizeof(network_name) - 1);
        network_name[sizeof(network_name) - 1] = '\0';
        trim_string(network_name);
        
        /* Parse fields after '=' */
        temp = equal_pos + 1;
        while (*temp && isspace((unsigned char)*temp)) temp++;
        
        /* Initialize fields */
        memset(fields, 0, sizeof(fields));
        field_count = parse_display_fields(temp, fields, 20); /* Increased to 20 fields */
        
        if (field_count < 2) {
            log_message("parse_network_section: WARNING - expected at least 2 fields, got %d in line: %s", field_count, line_copy);
            continue;
        }
        
        /* Process each field */
        driver_file[0] = '\0';
        description[0] = '\0';
        help_file[0] = '\0';
        opt_file[0] = '\0';
        winini_sect_name[0] = '\0';
        sysini_sect_name[0] = '\0';
        
        /* Clear VDD array */
        memset(vdd_array, 0, sizeof(vdd_array));
        vdd_count = 0;
        
        /* Copy and trim each field */
        if (fields[0]) {
            strncpy(driver_file, fields[0], sizeof(driver_file) - 1);
            driver_file[sizeof(driver_file) - 1] = '\0';
            trim_string(driver_file);
        }
        
        if (fields[1]) {
            strncpy(description, fields[1], sizeof(description) - 1);
            description[sizeof(description) - 1] = '\0';
            trim_string(description);
        }
        
        if (fields[2]) {
            strncpy(help_file, fields[2], sizeof(help_file) - 1);
            help_file[sizeof(help_file) - 1] = '\0';
            trim_string(help_file);
        }
        
        if (fields[3]) {
            strncpy(opt_file, fields[3], sizeof(opt_file) - 1);
            opt_file[sizeof(opt_file) - 1] = '\0';
            trim_string(opt_file);
        }
        
        if (fields[4]) {
            strncpy(winini_sect_name, fields[4], sizeof(winini_sect_name) - 1);
            winini_sect_name[sizeof(winini_sect_name) - 1] = '\0';
            trim_string(winini_sect_name);
        }
        
        if (fields[5]) {
            strncpy(sysini_sect_name, fields[5], sizeof(sysini_sect_name) - 1);
            sysini_sect_name[sizeof(sysini_sect_name) - 1] = '\0';
            trim_string(sysini_sect_name);
        }
        
        /* Process VDD fields (fields 6 and beyond) */
        for (j = 6; j < field_count && vdd_count < MAX_VDD_COUNT; j++) {
            if (fields[j] && fields[j][0] != '\0') {
                char vdd_temp[100];
                strncpy(vdd_temp, fields[j], sizeof(vdd_temp) - 1);
                vdd_temp[sizeof(vdd_temp) - 1] = '\0';
                trim_string(vdd_temp);
                
                if (strlen(vdd_temp) > 0) {
                    vdd_array[vdd_count] = (char*)malloc(strlen(vdd_temp) + 1);
                    if (vdd_array[vdd_count]) {
                        strcpy(vdd_array[vdd_count], vdd_temp);
                        vdd_count++;
                    }
                }
            }
        }
        
        log_message("parse_network_section: Parsed network '%s':", network_name);
        log_message("  driver_file: '%s'", driver_file);
        log_message("  description: '%s'", description);
        log_message("  help_file: '%s'", help_file);
        log_message("  opt_file: '%s'", opt_file);
        log_message("  winini_sect_name: '%s'", winini_sect_name);
        log_message("  sysini_sect_name: '%s'", sysini_sect_name);
        log_message("  vdd_count: %d", vdd_count);
        for (j = 0; j < vdd_count; j++) {
            log_message("  vdd[%d]: '%s'", j, vdd_array[j]);
        }
        
        /* Add to network list */
        network_list_add(network_name, driver_file, description, help_file, opt_file,
                       winini_sect_name, sysini_sect_name, vdd_array, vdd_count);
        
        /* Free temporary VDD array memory */
        for (j = 0; j < vdd_count; j++) {
            if (vdd_array[j]) free(vdd_array[j]);
        }
    }
    
    _ffree(lines);
    
    /* Remove network section after parsing to free memory */
    inf_section_remove("network");
    log_message("parse_network_section: Network section removed after processing");
    
    log_message("parse_network_section: Network section parsing completed, found %d entries", network_list_count());
}

/* UPDATED: Preparse INF file function - added parse_network_section call */
void preparse_inf_file(SetupConfig* config) {
    log_message("preparse_inf_file: Starting INF file pre-parsing");
    
    if (!config) {
        log_message("preparse_inf_file: ERROR - null config");
        return;
    }
    
    log_message("preparse_inf_file: Using INF file: %s", config->inf_filename);
    
    /* Parse disks section */
    parse_disks_section();
    
    /* Parse machine section */
    parse_machine_section();
    
    /* Parse display section */
    parse_display_section();
    
    /* Parse mouse section */
    parse_mouse_section();
    
    /* Parse keyboard type section */
    parse_kbdtype_section();
    
    /* Parse keyboard table section */
    parse_kbdlay_section();
    
    /* Parse language section */
    parse_language_section();
    
    /* NEW: Parse network section */
    parse_network_section();
    
    log_message("preparse_inf_file: INF file pre-parsing completed");
}

/* Helper function to check if file should be copied based on condition */
static int should_copy_file(const char far *condition, const SetupConfig* config) {
    unsigned int cond_len;
    char *near_condition;
    int result;

    if (condition == NULL || condition[0] == '\0') {
        return 1; /* No condition - always copy */
    }
    
    /* Convert far condition to near string for comparison */
    cond_len = _fstrlen(condition);
    near_condition = (char*)malloc(cond_len + 1);
    if (near_condition) {
        _fstrcpy(near_condition, condition);
    } else {
        return 0; /* Out of memory - skip file */
    }
    
    result = 0;
    
    if (strcmp(near_condition, "Net") == 0) {
        /* Copy only for administrative setup */
        result = config->admin_setup;
        log_message("File condition 'Net': copying only for admin setup (admin_setup=%d)", config->admin_setup);
    } else {
        /* Copy if condition matches selected network */
        result = (strcmp(near_condition, config->selected_network) == 0);
        log_message("File condition '%s': copying only for network '%s' (selected_network=%s)", 
                   near_condition, near_condition, config->selected_network);
    }
    
    free(near_condition);
    return result;
}

/* Simple parser for file copy sections */
static void parse_file_copy_line(const char* line, char* source, char* condition) {
    char line_copy[256];
    char *comma_pos;
    char *temp;
    char *end_ptr;
    
    source[0] = '\0';
    condition[0] = '\0';
    
    if (!line || strlen(line) == 0) return;
    
    /* Make a working copy */
    strncpy(line_copy, line, sizeof(line_copy) - 1);
    line_copy[sizeof(line_copy) - 1] = '\0';
    
    /* Skip empty lines and comments */
    temp = line_copy;
    while (*temp && isspace((unsigned char)*temp)) temp++;
    if (*temp == '\0' || *temp == ';') return;
    
    /* Trim the line */
    end_ptr = line_copy + strlen(line_copy) - 1;
    while (end_ptr > line_copy && isspace((unsigned char)*end_ptr)) {
        *end_ptr = '\0';
        end_ptr--;
    }
    
    /* Find comma separator for condition */
    comma_pos = strchr(line_copy, ',');
    
    if (comma_pos) {
        /* Split into source and condition */
        *comma_pos = '\0';
        
        /* Trim source part */
        temp = line_copy;
        while (*temp && isspace((unsigned char)*temp)) temp++;
        end_ptr = temp + strlen(temp) - 1;
        while (end_ptr > temp && isspace((unsigned char)*end_ptr)) {
            *end_ptr = '\0';
            end_ptr--;
        }
        strcpy(source, temp);
        
        /* Trim condition part */
        temp = comma_pos + 1;
        while (*temp && isspace((unsigned char)*temp)) temp++;
        end_ptr = temp + strlen(temp) - 1;
        while (end_ptr > temp && isspace((unsigned char)*end_ptr)) {
            *end_ptr = '\0';
            end_ptr--;
        }
        strcpy(condition, temp);
    } else {
        /* No condition - just source */
        temp = line_copy;
        while (*temp && isspace((unsigned char)*temp)) temp++;
        end_ptr = temp + strlen(temp) - 1;
        while (end_ptr > temp && isspace((unsigned char)*end_ptr)) {
            *end_ptr = '\0';
            end_ptr--;
        }
        strcpy(source, temp);
    }
}

void add_setup_files(const SetupConfig* config) {
    log_message("add_setup_files: Adding setup files to copy list");
    
    if (!config) {
        log_message("add_setup_files: ERROR - null config");
        return;
    }
    
    /* Add SETUP.EXE file - disk 1 */
    file_list_add_entry('1', "SETUP.EXE", config->windowsDir, "SETUP.EXE");
    
    /* Add INF file - pseudo disk -1 with full source path */
    file_list_add_entry(-1, config->inf_filename, config->systemDir, "SETUP.INF");
    
    log_message("add_setup_files: Setup files added to copy list");
}

void add_section_files(const char* section_name, const char* dest_dir, const SetupConfig* config) {
    char far * far *lines;
    int line_count;
    int i;
    char line_copy[256];
    char source[256];
    char condition[256];
    char filename_only[256];
    char dest_filename[256];
    char disk_char;
    unsigned int line_len;
    
    if (inf_section_find((const char far *)section_name, &lines, &line_count)) {
        log_message("add_section_files: Adding files from [%s] section to %s directory", section_name, dest_dir);
        for (i = 0; i < line_count; i++) {
            if (!lines[i]) continue;
            
            /* Parse the line for file copy sections */
            source[0] = '\0';
            condition[0] = '\0';
            
            /* Safely copy far string to near buffer */
            line_len = _fstrlen(lines[i]);
            if (line_len >= sizeof(line_copy)) {
                line_len = sizeof(line_copy) - 1;
            }
            _fstrncpy(line_copy, lines[i], line_len);
            line_copy[line_len] = '\0';
            
            parse_file_copy_line(line_copy, source, condition);
            
            if (source[0] != '\0') {
                /* Check condition */
                if (condition[0] != '\0' && !should_copy_file((const char far*)condition, config)) {
                    log_message("add_section_files: Skipping file due to condition '%s': %s", condition, source);
                    continue; /* Skip file due to condition */
                }
                
                /* Extract disk number and filename from source */
                disk_char = '1'; /* Default disk */
                strcpy(filename_only, source);
                strcpy(dest_filename, source);
                
                /* Remove disk prefix from source and destination names */
                remove_disk_prefix(filename_only);
                remove_disk_prefix(dest_filename);
                
                /* Extract actual disk number if present */
                if (strchr(source, ':') != NULL) {
                    disk_char = source[0];
                    if (disk_char < '0' || disk_char > '9') {
                        disk_char = '1'; /* Invalid disk, use default */
                    }
                }
                
                /* Add to file list */
                file_list_add_entry(disk_char, filename_only, dest_dir, dest_filename);
                
                log_message("add_section_files: Added file - disk: %c, source: '%s', dest: '%s'", 
                           disk_char, filename_only, dest_filename);
            }
        }
        _ffree(lines);
        
        /* Remove the section after processing to free memory */
        inf_section_remove((const char far *)section_name);
        log_message("add_section_files: Section [%s] removed after processing", section_name);
    } else {
        log_message("add_section_files: Warning: [%s] section not found in INF file", section_name);
    }
}

void parse_inf_file(const SetupConfig* config) {
    log_message("parse_inf_file: Starting INF file parsing");
    
    if (!config) {
        log_message("parse_inf_file: ERROR - null config");
        return;
    }
    
    /* Add setup files to copy list */
    add_setup_files(config);
    
    /* Add files from windows.system section */
    add_section_files("windows.system", config->systemDir, config);
    
    /* Add files from windows section */
    add_section_files("windows", config->windowsDir, config);
    
    /* Add CPU-specific sections for 386 */
    if (config->cpu_type == 2) { /* CPU_386 */
        add_section_files("windows.system.386", config->systemDir, config);
    } else {
        /* Remove windows.system.386 section if not 386 CPU to free memory */
        log_message("parse_inf_file: Not 386 CPU, removing windows.system.386 section");
        inf_section_remove("windows.system.386");
    }
    
    /* Dump file list contents for verification */
    file_list_dump();
    
    log_message("parse_inf_file: INF file parsing completed");
}
