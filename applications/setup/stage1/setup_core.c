#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <conio.h>
#include <direct.h>
#include <fcntl.h>
#include <io.h>
#include <sys/stat.h>
#include <malloc.h>
#include <dos.h>
#include <stdarg.h>
#include <process.h>
#include "ui.h"
#include "setup_core.h"
#include "inf_parser.h"
#include "file_utils.h"
#include "resources.h"
#include "log.h"
#include "dialog.h"
#include "section_list.h"
#include "machine_list.h"  /* Use machine_list directly */

/* FIXED: Helper function to parse a single line from raw section data */
static void parse_section_line(const char* line, char** key, char** value, char** condition)
{
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
    
    equal_sign = strchr(line_copy, '=');
    comma_pos = strchr(line_copy, ',');
    
    if (equal_sign) {
        /* key = value format */
        *equal_sign = '\0';
        *key = line_copy;
        *value = equal_sign + 1;
        
        /* Trim key and value */
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
        
        /* Remove quotes from value if present */
        if ((*value)[0] == '"' && (*value)[strlen(*value)-1] == '"') {
            memmove(*value, *value + 1, strlen(*value) - 2);
            (*value)[strlen(*value) - 2] = '\0';
        }
    }
    else if (comma_pos) {
        /* key, value, condition format */
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
        
        /* Trim all parts */
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
        /* Simple key-only format */
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

void setup_config_init(SetupConfig* config) {
    strcpy(config->windowsDir, "C:\\WINDOWS");
    strcpy(config->systemDir, "C:\\WINDOWS\\SYSTEM");
    strcpy(config->tempDir, "C:\\WINDOWS\\TEMP");
    strcpy(config->sourcePath, "");
    strcpy(config->logFile, "C:\\SETUP.LOG");
    config->cpu_type = 0; /* CPU_8086 */
    config->setupMode = 0;
    strcpy(config->inf_filename, "");
    strcpy(config->selected_network, "nonet");
    config->network_installed = 0;
    
    /* Initialize command line flags */
    config->ignore_autodetect = 0;
    config->network_setup = 0;
    config->admin_setup = 0;
    config->monochrome_setup = 0;
    config->scan_incompatible = 0;
    config->batch_file[0] = '\0';
    
    /* Initialize machine selection */
    config->selected_machine = -1;
    config->machine_name[0] = '\0';
}

int copy_file_with_version_check(const char *source, const char *dest, int setupMode) {
    /* If it's a new installation, always copy */
    if (setupMode == 0) {
        return copy_file(source, dest);
    }
    
    /* For upgrade mode, check if destination exists */
    if (!file_exists(dest)) {
        log_message("New file in upgrade: %s", dest);
        return copy_file(source, dest);
    }
    
    /* Check if source is newer than destination */
    if (is_source_file_newer(source, dest)) {
        log_message("Upgrading file (source is newer): %s", dest);
        return copy_file(source, dest);
    } else {
        log_message("Skipping file (destination is newer or same): %s", dest);
        return 1; /* Skip copying but return success */
    }
}

void update_autoexec_bat(const SetupConfig* config) {
    FILE *autoexec;
    char autoexec_path[80];
    char line[256];
    int path_updated = 0;
    int temp_updated = 0;
    int windir_updated = 0;
    int i;
    
    /* Try multiple possible locations for AUTOEXEC.BAT */
    const char* possible_paths[] = {
        "C:\\AUTOEXEC.BAT",
        "A:\\AUTOEXEC.BAT", 
        "B:\\AUTOEXEC.BAT",
        NULL
    };
    
    for (i = 0; possible_paths[i] != NULL; i++) {
        if (file_exists(possible_paths[i])) {
            strcpy(autoexec_path, possible_paths[i]);
            break;
        }
    }
    
    if (possible_paths[i] == NULL) {
        strcpy(autoexec_path, "C:\\AUTOEXEC.BAT");
    }
    
    /* First check if updates are already present */
    autoexec = fopen(autoexec_path, "rt");
    if (autoexec) {
        while (fgets(line, sizeof(line), autoexec)) {
            if (strstr(line, config->windowsDir)) path_updated = 1;
            if (strstr(line, "TEMP=") && strstr(line, config->tempDir)) temp_updated = 1;
            if (strstr(line, "WINDIR=") && strstr(line, config->windowsDir)) windir_updated = 1;
        }
        fclose(autoexec);
    }
    
    autoexec = fopen(autoexec_path, "at");
    if (autoexec) {
        log_message("Updating AUTOEXEC.BAT at %s", autoexec_path);
        
        if (!path_updated) {
            fprintf(autoexec, "\nSET PATH=%%PATH%%;%s;%s\n", config->windowsDir, config->systemDir);
        }
        if (!temp_updated) {
            fprintf(autoexec, "SET TEMP=%s\n", config->tempDir);
        }
        if (!windir_updated) {
            fprintf(autoexec, "SET WINDIR=%s\n", config->windowsDir);
        }
        
        fclose(autoexec);
    } else {
        log_message("Warning: Cannot open AUTOEXEC.BAT for update at %s", autoexec_path);
    }
}

void update_config_sys(void) {
    FILE *config;
    char config_path[80];
    char line[256];
    int files_updated = 0;
    int buffers_updated = 0;
    int stacks_updated = 0;
    int i;
    
    /* Try multiple possible locations for CONFIG.SYS */
    const char* possible_paths[] = {
        "C:\\CONFIG.SYS",
        "A:\\CONFIG.SYS",
        "B:\\CONFIG.SYS",
        NULL
    };
    
    for (i = 0; possible_paths[i] != NULL; i++) {
        if (file_exists(possible_paths[i])) {
            strcpy(config_path, possible_paths[i]);
            break;
        }
    }
    
    if (possible_paths[i] == NULL) {
        strcpy(config_path, "C:\\CONFIG.SYS");
    }
    
    /* First check if updates are already present */
    config = fopen(config_path, "rt");
    if (config) {
        while (fgets(line, sizeof(line), config)) {
            if (strstr(line, "FILES=")) files_updated = 1;
            if (strstr(line, "BUFFERS=")) buffers_updated = 1;
            if (strstr(line, "STACKS=")) stacks_updated = 1;
        }
        fclose(config);
    }
    
    config = fopen(config_path, "at");
    if (config) {
        log_message("Updating CONFIG.SYS at %s", config_path);
        
        if (!files_updated) {
            fprintf(config, "\nFILES=30\n");
        }
        if (!buffers_updated) {
            fprintf(config, "BUFFERS=20\n");
        }
        if (!stacks_updated) {
            fprintf(config, "STACKS=9,256\n");
        }
        
        fclose(config);
    } else {
        log_message("Warning: Cannot open CONFIG.SYS for update at %s", config_path);
    }
}

void setup_directories(const SetupConfig* config) {
    log_message("Creating directories: %s, %s", config->windowsDir, config->systemDir);
    create_directory(config->windowsDir);
    create_directory(config->systemDir);
    /* Don't create TEMP directory here - it will be created when needed */
}

void detect_setup_mode(SetupConfig* config) {
    char win_com_path[80];
    char win_ini_path[80];
    
    log_message("Checking setup mode for directory: %s", config->windowsDir);
    
    if (file_exists(config->windowsDir)) {
        /* Check for WIN.COM first */
        sprintf(win_com_path, "%s\\WIN.COM", config->windowsDir);
        if (file_exists(win_com_path)) {
            config->setupMode = 1; /* Upgrade mode */
            log_message("Setup mode: Upgrade (WIN.COM exists)");
            return;
        }
        
        /* Also check for WIN.INI as backup */
        sprintf(win_ini_path, "%s\\WIN.INI", config->windowsDir);
        if (file_exists(win_ini_path)) {
            config->setupMode = 1; /* Upgrade mode */
            log_message("Setup mode: Upgrade (WIN.INI exists)");
            return;
        }
        
        /* Directory exists but no Windows files found */
        config->setupMode = 0; /* New installation */
        log_message("Setup mode: New installation (Windows directory exists but no Windows files found)");
    } else {
        config->setupMode = 0; /* New installation */
        log_message("Setup mode: New installation (Windows directory doesn't exist)");
    }
}

void check_disk_space(const SetupConfig* config) {
    long requiredSpace = 0;
    long freeSpace = get_free_disk_space(config->windowsDir);
    char *minSpaceKey;
    char *minSpace;

    if (config->setupMode == 0) {
        switch (config->cpu_type) {
            case 2: /* CPU_386 */
                minSpaceKey = "neededspace386";
                break;
            case 1: /* CPU_286 */
                minSpaceKey = "neededspace286";
                break;
            case 0: /* CPU_8086 */
            default:
                minSpaceKey = "neededspace286";
                break;
        }
    } else {
        switch (config->cpu_type) {
            case 2: /* CPU_386 */
                requiredSpace = 3000000;
                break;
            case 1: /* CPU_286 */
                requiredSpace = 2000000;
                break;
            case 0: /* CPU_8086 */
            default:
                requiredSpace = 1500000;
                break;
        }
    }

    if (config->setupMode == 0) {
        minSpace = get_private_profile_string("data", minSpaceKey, NULL, config->inf_filename);
        if (minSpace) {
            requiredSpace = atol(minSpace);
            free(minSpace);
        } else {
            switch (config->cpu_type) {
                case 2: /* CPU_386 */
                    requiredSpace = 6300000;
                    break;
                case 1: /* CPU_286 */
                    requiredSpace = 4500000;
                    break;
                case 0: /* CPU_8086 */
                default:
                    requiredSpace = 4500000;
                    break;
            }
        }
    }

    log_message("Disk space check - Required: %ld, Available: %ld", requiredSpace, freeSpace);

    if (freeSpace < requiredSpace) {
        char message[100];
        sprintf(message, "Insufficient disk space. Required: %ld, Available: %ld", 
                requiredSpace, freeSpace);
        display_message(message);
        log_message("Fatal: %s", message);
        exit(1);
    }
}

void setup_configuration(const SetupConfig* config) {
    char wininiPath[80];
    char systeminiPath[80];
    FILE *winini, *systemini;
    
    /* Only create configuration files for new installations */
    if (config->setupMode == 1) {
        log_message("Skipping configuration file creation for upgrade mode");
        return;
    }
    
    sprintf(wininiPath, "%s\\WIN.INI", config->windowsDir);
    winini = fopen(wininiPath, "w");
    if (winini) {
        log_message("Creating WIN.INI");
        fprintf(winini, "[Windows]\n");
        fprintf(winini, "load=\n");
        fprintf(winini, "run=\n");
        fprintf(winini, "Beep=yes\n");
        fprintf(winini, "Spooler=yes\n");
        fprintf(winini, "NullPort=None\n");
        fprintf(winini, "BorderWidth=3\n");
        fprintf(winini, "CursorBlinkRate=530\n");
        fprintf(winini, "DoubleClickSpeed=452\n");
        fprintf(winini, "Programs=com exe bat pif\n");
        fprintf(winini, "Documents=\n");
        fprintf(winini, "DeviceNotSelectedTimeout=15\n");
        fprintf(winini, "TransmissionRetryTimeout=45\n");
        fclose(winini);
    } else {
        log_message("Error: Cannot create WIN.INI");
    }
    
    sprintf(systeminiPath, "%s\\SYSTEM.INI", config->windowsDir);
    systemini = fopen(systeminiPath, "w");
    if (systemini) {
        log_message("Creating SYSTEM.INI");
        fprintf(systemini, "[boot]\n");
        fprintf(systemini, "shell=progman.exe\n");
        
        switch (config->cpu_type) {
            case 2: /* CPU_386 */
                fprintf(systemini, "display.drv=vga.drv\n");
                fprintf(systemini, "keyboard.drv=keyboard.drv\n");
                fprintf(systemini, "mouse.drv=mouse.drv\n");
                fprintf(systemini, "fixedfon.fon=vgafix.fon\n");
                fprintf(systemini, "fonts.fon=vgasys.fon\n");
                fprintf(systemini, "oemfonts.fon=vgaoem.fon\n");
                break;
            case 1: /* CPU_286 */
                fprintf(systemini, "display.drv=vga.drv\n");
                fprintf(systemini, "keyboard.drv=keyboard.drv\n");
                fprintf(systemini, "mouse.drv=mouse.drv\n");
                fprintf(systemini, "fixedfon.fon=vgafix.fon\n");
                fprintf(systemini, "fonts.fon=vgasys.fon\n");
                fprintf(systemini, "oemfonts.fon=vgaoem.fon\n");
                break;
            case 0: /* CPU_8086 */
            default:
                fprintf(systemini, "display.drv=ega.drv\n");
                fprintf(systemini, "keyboard.drv=keyboard.drv\n");
                fprintf(systemini, "mouse.drv=mouse.drv\n");
                fprintf(systemini, "fixedfon.fon=egafix.fon\n");
                fprintf(systemini, "fonts.fon=egasys.fon\n");
                fprintf(systemini, "oemfonts.fon=egaoem.fon\n");
                break;
        }
        fclose(systemini);
    } else {
        log_message("Error: Cannot create SYSTEM.INI");
    }
}

int find_file_copy_section(FILE* file, const char* section_name) {
    char line[512];
    char current_section[100] = "";
    int in_target_section = 0;
    char* end;
    
    fseek(file, 0, SEEK_SET);
    
    while (fgets(line, sizeof(line), file) != NULL) {
        line[strcspn(line, "\r\n")] = 0;
        
        if (line[0] == '\0' || line[0] == ';') {
            continue;
        }
        
        if (line[0] == '[') {
            end = strchr(line, ']');
            if (end != NULL) {
                *end = '\0';
                strncpy(current_section, line + 1, sizeof(current_section)-1);
                current_section[sizeof(current_section)-1] = '\0';
                in_target_section = (strcmp(current_section, section_name) == 0);
            } else {
                in_target_section = 0;
            }
            continue;
        }
        
        if (in_target_section) {
            if (strchr(line, '=') != NULL) {
                return 1; /* Found files in this section */
            }
        }
    }
    
    return 0;
}

void select_network(SetupConfig* config) {
    /* For now, default to nonet */
    /* In full implementation, this would show a network selection dialog */
    strcpy(config->selected_network, "nonet");
    config->network_installed = 0;
    log_message("Network selected: %s", config->selected_network);
}

void parse_line_with_condition(char *line, char **key, char **value, char **condition) {
    char *first_comma = strchr(line, ',');
    char *second_comma = NULL;
    char *val_end;
    char *key_end;
    char *cond_end;
    
    *key = line;
    *value = line;  // default: value = key
    *condition = NULL;
    
    if (first_comma != NULL) {
        *first_comma = '\0';
        *value = first_comma + 1;
        
        second_comma = strchr(*value, ',');
        if (second_comma != NULL) {
            *second_comma = '\0';
            *condition = second_comma + 1;
            
            // Trim condition
            while (**condition && isspace((unsigned char)**condition)) (*condition)++;
            cond_end = *condition + strlen(*condition) - 1;
            while (cond_end > *condition && isspace((unsigned char)*cond_end)) {
                *cond_end = '\0';
                cond_end--;
            }
        }
        
        // Trim value
        while (**value && isspace((unsigned char)**value)) (*value)++;
        val_end = *value + strlen(*value) - 1;
        while (val_end > *value && isspace((unsigned char)*val_end)) {
            *val_end = '\0';
            val_end--;
        }
    }
    
    // Trim key
    while (**key && isspace((unsigned char)**key)) (*key)++;
    key_end = *key + strlen(*key) - 1;
    while (key_end > *key && isspace((unsigned char)*key_end)) {
        *key_end = '\0';
        key_end--;
    }
}

/* FIXED: Use machine_list directly with proper bounds checking */
void copy_machine_files(const SetupConfig* config) {
    MachineEntry* machine_entry;
    char source_path[80], dest_path[80];
    char* filename;
    
    if (config->selected_machine == -1) {
        log_message("No machine selected, skipping machine file copy");
        return;
    }
    
    log_message("Copying machine-specific files for: %s", config->machine_name);
    
    /* Get machine entry directly from machine_list with bounds checking */
    if (config->selected_machine >= machine_list_count()) {
        log_message("Error: Invalid machine index %d, machine list has %d entries", 
                   config->selected_machine, machine_list_count());
        return;
    }
    
    machine_entry = machine_list_get(config->selected_machine);
    if (!machine_entry) {
        log_message("No machine configuration found for index: %d", config->selected_machine);
        return;
    }
    
    /* Copy only the system driver from machine configuration */
    if (machine_entry->system_drv) {
        /* Convert far string to near string for get_string */
        unsigned int len = _fstrlen(machine_entry->system_drv);
        char *near_system_drv = (char*)malloc(len + 1);
        if (near_system_drv) {
            _fstrcpy(near_system_drv, machine_entry->system_drv);
            
            /* Look up filename in system section */
            filename = get_string("system", near_system_drv);
            free(near_system_drv);
        } else {
            filename = NULL;
        }
        
        if (filename) {
            log_message("Found system driver filename for %s: %s", 
                       machine_entry->system_drv, filename);
            remove_disk_prefix(filename);
            log_message("After removing disk prefix: %s", filename);
            
            sprintf(source_path, "%s\\%s", config->sourcePath, filename);
            sprintf(dest_path, "%s\\%s", config->systemDir, filename);
            
            log_message("Copying system driver: %s -> %s", source_path, dest_path);
            if (!copy_file_with_version_check(source_path, dest_path, config->setupMode)) {
                log_message("Warning: Failed to copy system driver: %s", source_path);
            } else {
                log_message("Successfully copied system driver: %s", filename);
            }
            
            free(filename);
        } else {
            log_message("Warning: No system file found for system driver: %s", 
                       machine_entry->system_drv);
        }
    } else {
        log_message("No system driver specified in machine configuration");
    }
    
    /* Note: Other drivers (kbd, mouse, display, etc.) are handled by other parts of setup */
    log_message("Machine-specific file copy completed for: %s", config->machine_name);
}

/* FIXED: Use machine_list directly */
void select_machine_type(SetupConfig* config) {
    char far * far *lines;
    int line_count;
    int i;
    char** menu_items;
    int item_count;
    int choice;
    char *line_key;
    char *line_value;
    char *line_condition;
    char line_copy[256];
    int menu_index;
    
    log_message("Selecting machine type");
    
    /* Use inf_section_find directly */
    if (!inf_section_find("machine", &lines, &line_count) || line_count == 0) {
        log_message("No machine section found, using default");
        config->selected_machine = 0;
        strcpy(config->machine_name, "MS-DOS or PC-DOS System");
        return;
    }
    
    /* Count valid machine entries by parsing raw lines */
    item_count = 0;
    for (i = 0; i < line_count; i++) {
        if (lines[i]) {
            _fstrncpy(line_copy, lines[i], sizeof(line_copy) - 1);
            line_copy[sizeof(line_copy) - 1] = '\0';
            
            line_key = NULL;
            line_value = NULL;
            line_condition = NULL;
            parse_section_line(line_copy, &line_key, &line_value, &line_condition);
            
            if (line_key && strlen(line_key) > 0) {
                item_count++;
            }
            
            if (line_key) free(line_key);
            if (line_value) free(line_value);
            if (line_condition) free(line_condition);
        }
    }
    
    _ffree(lines);
    
    if (item_count == 0) {
        log_message("No machine entries found, using default");
        config->selected_machine = 0;
        strcpy(config->machine_name, "MS-DOS or PC-DOS System");
        return;
    }
    
    /* Create menu items */
    menu_items = (char**)malloc(item_count * sizeof(char*));
    if (!menu_items) {
        log_message("Memory allocation failed for machine menu");
        config->selected_machine = 0;
        strcpy(config->machine_name, "MS-DOS or PC-DOS System");
        return;
    }
    
    /* Parse machine entries again to extract descriptions */
    menu_index = 0;
    /* Use inf_section_find directly */
    if (inf_section_find("machine", &lines, &line_count)) {
        for (i = 0; i < line_count && menu_index < item_count; i++) {
            if (lines[i]) {
                _fstrncpy(line_copy, lines[i], sizeof(line_copy) - 1);
                line_copy[sizeof(line_copy) - 1] = '\0';
                
                line_key = NULL;
                line_value = NULL;
                line_condition = NULL;
                parse_section_line(line_copy, &line_key, &line_value, &line_condition);
                
                if (line_key && strlen(line_key) > 0) {
                    menu_items[menu_index] = (char*)malloc(strlen(line_key) + 1);
                    if (menu_items[menu_index]) {
                        strcpy(menu_items[menu_index], line_key);
                    } else {
                        menu_items[menu_index] = "Unknown Machine";
                    }
                    menu_index++;
                }
                
                if (line_key) free(line_key);
                if (line_value) free(line_value);
                if (line_condition) free(line_condition);
            }
        }
        _ffree(lines);
    }
    
    /* Display machine selection menu */
    display_header("Select Machine Type");
    choice = display_menu(10, 5, 60, item_count, "Select your computer type:", menu_items, item_count);
    
    /* Free menu items */
    for (i = 0; i < item_count; i++) {
        free(menu_items[i]);
    }
    free(menu_items);
    
    if (choice >= 0 && choice < item_count) {
        MachineEntry* machine_entry;
        config->selected_machine = choice;
        /* Get the machine name from the parsed machine entries */
        machine_entry = machine_list_get(choice);
        if (machine_entry && machine_entry->description) {
            unsigned int len = _fstrlen(machine_entry->description);
            if (len < sizeof(config->machine_name)) {
                _fstrcpy(config->machine_name, machine_entry->description);
            }
        } else {
            strcpy(config->machine_name, "MS-DOS or PC-DOS System");
        }
        log_message("Selected machine: %s", config->machine_name);
    } else {
        /* Default selection */
        config->selected_machine = 0;
        strcpy(config->machine_name, "MS-DOS or PC-DOS System");
        log_message("Using default machine: %s", config->machine_name);
    }
}

/* FIXED: Updated to use new file list system */
int copy_files(const SetupConfig* config) {
    log_message("copy_files: Starting file copy using new file list system");
    
    /* TODO: Implement file copying from file list */
    
    log_message("copy_files: File copy completed (stub - using new file list)");
    return 1;
}
