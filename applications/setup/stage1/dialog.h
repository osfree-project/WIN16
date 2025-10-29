#ifndef DIALOG_H
#define DIALOG_H

int display_yesno(const char *question);
void display_message(const char *message);
int display_file_error(const char* filename);
int display_error_dialog(const char* line1, const char* line2, const char* line3);
int display_exit_confirmation(void);

#endif
