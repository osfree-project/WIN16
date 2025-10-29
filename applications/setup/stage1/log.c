#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "log.h"

static FILE *log_handle = NULL;

void log_init(const char* filename)
{
    if (log_handle) {
        fclose(log_handle);
    }
    
    log_handle = fopen(filename, "w");
    if (log_handle) {
        setbuf(log_handle, NULL);
    }
}

void log_message(const char* format, ...)
{
    if (log_handle) {
        va_list args;
        va_start(args, format);
        vfprintf(log_handle, format, args);
        fprintf(log_handle, "\n");
        fflush(log_handle);
        va_end(args);
    }
}

void log_close(void)
{
    if (log_handle) {
        fclose(log_handle);
        log_handle = NULL;
    }
}

FILE* get_log_handle(void)
{
    return log_handle;
}
