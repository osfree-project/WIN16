#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "setup_core.h"
#include "resources.h"
#include "system_info.h"
#include "file_utils.h"
#include "log.h"
#include "ui.h"
#include "dialog.h"
#include "screens.h"
#include "section_list.h"
#include "inf_parser.h"

int launch_stage2(const SetupConfig* config) {
    char win_com_path[80];
    char command[512];
    
    /* Only try to launch Windows, not setup again */
    sprintf(win_com_path, "%s\\WIN.COM", config->windowsDir);
    
    if (file_exists(win_com_path)) {
        log_message("Launching Windows: %s", win_com_path);
        printf("\nSetup completed successfully. Launching Windows...\n");
        
        /* Use system call instead of spawn to avoid memory issues */
        sprintf(command, "%s", win_com_path);
        return system(command);
    }
    
    /* If WIN.COM not found, just exit successfully */
    log_message("Windows setup completed. WIN.COM not found, exiting.");
    printf("\nWindows setup completed successfully.\n");
    printf("You can now run Windows by typing WIN at the command prompt.\n");
    return 0;
}

void display_command_line_help(void) {
    printf("Windows Setup Command Line Switches:\n\n");
    printf("/?      Display this help message\n");
    printf("/i      Ignore automatic hardware detection\n");
    printf("/n      Set up a shared copy of Windows from a network server\n");
    printf("/a      Administrative Setup: copy all files to network server and mark as read-only\n");
    printf("/b      Set up Windows with monochrome display attributes\n");
    printf("/t      Search for incompatible software (for maintenance only)\n");
    printf("/h:filename Run Batch Mode Setup using the specified system settings file\n");
    printf("/o:filename Specify the SETUP.INF file\n");
    printf("/s:filename Specify the path for Windows installation disks\n\n");
}

void parse_command_line(int argc, char *argv[], SetupConfig* config) {
    int i;
    
    for (i = 1; i < argc; i++) {
        if (argv[i][0] == '/' || argv[i][0] == '-') {
            if (strlen(argv[i]) < 2) {
                log_message("Warning: Invalid command line switch: %s", argv[i]);
                continue;
            }
            
            switch (tolower(argv[i][1])) {
                case '?':
                    display_command_line_help();
                    exit(0);
                    break;
                    
                case 'i':
                    config->ignore_autodetect = 1;
                    log_message("Command line: /i - Ignore automatic hardware detection");
                    break;
                    
                case 'n':
                    config->network_setup = 1;
                    log_message("Command line: /n - Network setup");
                    break;
                    
                case 'a':
                    config->admin_setup = 1;
                    log_message("Command line: /a - Administrative setup");
                    break;
                    
                case 'b':
                    config->monochrome_setup = 1;
                    log_message("Command line: /b - Monochrome display");
                    break;
                    
                case 't':
                    config->scan_incompatible = 1;
                    log_message("Command line: /t - Scan for incompatible software");
                    break;
                    
                case 'h':
                    if (strlen(argv[i]) > 3 && argv[i][2] == ':') {
                        strncpy(config->batch_file, argv[i] + 3, 80 - 1);
                        config->batch_file[80-1] = '\0';
                        log_message("Command line: /h:%s - Batch mode setup", config->batch_file);
                    } else {
                        log_message("Warning: Invalid /h switch format: %s", argv[i]);
                    }
                    break;
                    
                case 'o':
                    if (strlen(argv[i]) > 3 && argv[i][2] == ':') {
                        strncpy(config->inf_filename, argv[i] + 3, 260 - 1);
                        config->inf_filename[260-1] = '\0';
                        log_message("Command line: /o:%s - Custom INF file", config->inf_filename);
                    } else {
                        log_message("Warning: Invalid /o switch format: %s", argv[i]);
                    }
                    break;
                    
                case 's':
                    if (strlen(argv[i]) > 3 && argv[i][2] == ':') {
                        strncpy(config->sourcePath, argv[i] + 3, 80 - 1);
                        config->sourcePath[80-1] = '\0';
                        log_message("Command line: /s:%s - Custom source path", config->sourcePath);
                    } else {
                        log_message("Warning: Invalid /s switch format: %s", argv[i]);
                    }
                    break;
                    
                default:
                    log_message("Warning: Unknown command line switch: %s", argv[i]);
                    break;
            }
        } else {
            /* Если аргумент без ключа, считаем его путем к исходным файлам */
            if (config->sourcePath[0] == '\0') {
                strncpy(config->sourcePath, argv[i], 80 - 1);
                config->sourcePath[80-1] = '\0';
                log_message("Command line: source path = %s", config->sourcePath);
            }
        }
    }
}

int main(int argc, char *argv[]) {
    SetupConfig config;
    int setup_type = 0;
    char temp_dir[80];
    
    /* Initialize configuration */
    setup_config_init(&config);
    
    /* Parse command line */
    parse_command_line(argc, argv, &config);
    
    /* Initialize logging */
    log_init(config.logFile);
    
    /* Initialize UI */
    init_ui();
    
    /* Display welcome screen */
    setup_welcome();

    /* Load INF file */
    if (config.inf_filename[0] == '\0') {
        strcpy(config.inf_filename, "SETUP.INF");
    }
    load_inf_file(config.inf_filename);

    /* Pre-parse SETUP.INF according config */
    preparse_inf_file(&config);
    
    /* Detect hardware */
    if (!config.ignore_autodetect) {
        config.cpu_type = detect_cpu_type();
        log_message("Detected CPU type: %d", config.cpu_type);
    }
    
    /* Display setup type menu */
    setup_type = setup_type_menu();
    
    /* Get installation directory */
    setup_directory_menu(config.windowsDir);
    
    /* Update system directories based on windows directory */
    sprintf(config.systemDir, "%s\\SYSTEM", config.windowsDir);
    sprintf(config.tempDir, "%s\\TEMP", config.windowsDir);
    
    /* Select network if needed */
    if (config.network_setup) {
        select_network(&config);
    }
    
    /* Detect setup mode */
    detect_setup_mode(&config);
    
    /* Check disk space */
    check_disk_space(&config);
    
    /* Display configuration confirmation */
    display_configuration_confirmation(config.windowsDir, 
                                      setup_type == 0 ? "Express" : 
                                      setup_type == 1 ? "Custom" : "Minimum");
    
    /* Setup directories */
    setup_directories(&config);
    
    /* Show loading screen */
    setup_loading_screen();

    /* parse SETUP.INF according config */
    parse_inf_file(&config);
    
    /* Copy files */
    if (!copy_files(&config)) {
        log_message("File copy failed");
        display_error_dialog("File copy failed.", "Please check the log file", "for details.");
        return 1;
    }
    
    /* Update configuration */
    update_autoexec_bat(&config);
    update_config_sys();
    setup_configuration(&config);
    
    /* Display completion screen */
    display_completion_screen();
    
    /* Launch stage 2 */
    launch_stage2(&config);
    
    /* Cleanup */
    free_resources();
    log_close();
    
    return 0;
}
