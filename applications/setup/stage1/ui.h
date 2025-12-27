#ifndef UI_H
#define UI_H

#include "video.h"

#define HEADER_BG LIGHTGRAY
#define HEADER_FG BLACK
#define MAIN_BG BLUE
#define MAIN_FG LIGHTGRAY
#define HIGHLIGHT_BG BLUE
#define HIGHLIGHT_FG WHITE
#define INPUT_BG LIGHTGRAY
#define INPUT_FG BLACK
#define HELP_BG LIGHTGRAY
#define HELP_FG BLACK
#define HELP_HEADER_BG LIGHTGRAY
#define HELP_HEADER_FG BLUE
#define HELP_FOOTER_BG BLUE
#define HELP_FOOTER_FG LIGHTGRAY

void init_ui(void);
void draw_header_footer(void);
void clear_screen(void);
void draw_box(int x, int y, int width, int height);
void center_text(int y, const char *text);
void display_header(const char *title);
void display_progress(int current, int total, const char *description);
int display_menu(int x, int y, int width, int height, const char *title, char **items, int item_count);
void setup_welcome(void);
int setup_type_menu(void);
void setup_directory_menu(char *directory);
int prompt_for_disk(char disk);
void setup_loading_screen(void);
void display_help(const char* topic);
void display_completion_screen(void);

/* Новые функции для статичных элементов экранов */
void draw_standard_header(void);
void draw_standard_footer(void);
void draw_main_content_area(void);

#endif
