#ifndef LOG_H
#define LOG_H

#include <stdio.h>
#include <stdarg.h>

void log_init(const char* filename);
void log_message(const char* format, ...);
void log_close(void);
FILE* get_log_handle(void);

#endif
