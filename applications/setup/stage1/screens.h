#ifndef SCREENS_H
#define SCREENS_H

void setup_welcome(void);
int setup_type_menu(void);
void setup_directory_menu(char *directory);
void display_configuration_confirmation(const char* windows_dir, const char* setup_type);
void display_completion_screen(void);
void setup_loading_screen(void);

#endif
