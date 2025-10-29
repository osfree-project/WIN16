#ifndef SETUP_CORE_H
#define SETUP_CORE_H


/* Configuration structure to encapsulate global state */
typedef struct {
    char windowsDir[80];
    char systemDir[80];
    char tempDir[80];
    char sourcePath[80];
    char logFile[80];
    int cpu_type;
    int setupMode;
    char inf_filename[260];
    char selected_network[50];
    int network_installed;
    
    /* Command line flags */
    int ignore_autodetect;
    int network_setup;
    int admin_setup;
    int monochrome_setup;
    int scan_incompatible;
    char batch_file[80];
    
    /* Machine selection */
    int selected_machine;
    char machine_name[50];
} SetupConfig;

/* Initialize configuration with default values */
void setup_config_init(SetupConfig* config);

/* File operations with setup context */
int copy_file_with_version_check(const char *source, const char *dest, int setupMode);

/* System configuration */
void update_autoexec_bat(const SetupConfig* config);
void update_config_sys(void);
void setup_directories(const SetupConfig* config);
void detect_setup_mode(SetupConfig* config);
void check_disk_space(const SetupConfig* config);
void setup_configuration(const SetupConfig* config);

/* File copying logic */
int find_file_copy_section(FILE* file, const char* section_name);
void remove_disk_prefix(char *str);
void parse_line_with_condition(char *line, char **key, char **value, char **condition);
int copy_files(const SetupConfig* config);

/* Machine selection and configuration */
void select_machine_type(SetupConfig* config);
void copy_machine_files(const SetupConfig* config);

#endif
