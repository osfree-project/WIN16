#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <conio.h>
#include <dos.h>
#include "ui.h"
#include "resources.h"
#include "help.h"
#include "video.h"
#include "dialog.h"

void init_ui(void) {
    set_video_mode(3);
    hide_cursor();
    clear_screen_direct();
}

/* Рисует стандартный заголовок */
void draw_standard_header(void) {
    int i;
    const char *title = "Windows Setup";
    
    put_string_at(1, 1, "Windows Setup", (MAIN_BG << 4) | MAIN_FG);
    put_string_at(0, 2, "\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd", (MAIN_BG << 4) | MAIN_FG);
}

/* Рисует стандартный футер */
void draw_standard_footer(void) {
    int i;
    const char *footer = "ENTER=Continue  F1=Help  F3=Exit";
    
    for (i = 0; i < SCREEN_WIDTH; i++) {
        put_char_at(i, SCREEN_HEIGHT - 1, ' ', (HEADER_BG << 4) | HEADER_FG);
    }
    
    put_string_at(3, SCREEN_HEIGHT - 1, footer, (HEADER_BG << 4) | HEADER_FG);
    put_string_at(50, SCREEN_HEIGHT - 1, "\xb3", (HEADER_BG << 4) | HEADER_FG);
}

/* Рисует основную область контента */
void draw_main_content_area(void) {
    int i, j;
    
    for (i = 1; i < SCREEN_HEIGHT - 1; i++) {
        for (j = 0; j < SCREEN_WIDTH; j++) {
            put_char_at(j, i, ' ', (MAIN_BG << 4) | MAIN_FG);
        }
    }
}

void draw_header_footer(void) {
    draw_standard_header();
    draw_standard_footer();
}

void clear_screen(void) {
    clear_screen_direct();
    draw_header_footer();
}

void draw_box(int x, int y, int width, int height) {
    int i, j;
    
    if (x < 0) x = 0;
    if (y < 0) y = 0;
    if (x + width > SCREEN_WIDTH) width = SCREEN_WIDTH - x;
    if (y + height > SCREEN_HEIGHT) height = SCREEN_HEIGHT - y;
    if (width <= 0 || height <= 0) return;
    
    put_char_at(x, y, 218, (WHITE << 4) | BLACK);
    for (i = 1; i < width - 1; i++) {
        put_char_at(x + i, y, 196, (WHITE << 4) | BLACK);
    }
    put_char_at(x + width - 1, y, 191, (WHITE << 4) | BLACK);
    
    for (i = y + 1; i < y + height - 1; i++) {
        put_char_at(x, i, 179, (WHITE << 4) | BLACK);
        for (j = x + 1; j < x + width - 1; j++) {
            put_char_at(j, i, ' ', (WHITE << 4) | BLACK);
        }
        put_char_at(x + width - 1, i, 179, (WHITE << 4) | BLACK);
    }
    
    put_char_at(x, y + height - 1, 192, (WHITE << 4) | BLACK);
    for (i = 1; i < width - 1; i++) {
        put_char_at(x + i, y + height - 1, 196, (WHITE << 4) | BLACK);
    }
    put_char_at(x + width - 1, y + height - 1, 217, (WHITE << 4) | BLACK);
}

void center_text(int y, const char *text) {
    int len;
    int x;
    
    len = strlen(text);
    if (len > SCREEN_WIDTH) {
        len = SCREEN_WIDTH;
    }
    x = (SCREEN_WIDTH - len) / 2;
    put_string_at(x, y, text, (MAIN_BG << 4) | MAIN_FG);
}

void display_header(const char *title) {
    int i;
    
    for (i = 0; i < SCREEN_WIDTH; i++) {
        put_char_at(i, 0, ' ', (HEADER_BG << 4) | HEADER_FG);
    }
    
    put_string_at((SCREEN_WIDTH - strlen(title)) / 2, 0, title, (HEADER_BG << 4) | HEADER_FG);
}

void display_progress(int current, int total, const char *description) {
    int y = 10;
    int barWidth = 50;
    int x;
    int percent;
    int bars;
    char percentStr[10];
    int i, j;
    
    x = (SCREEN_WIDTH - barWidth) / 2;
    percent = total > 0 ? (current * 100) / total : 0;
    bars = (percent * barWidth) / 100;
    
    for (i = 0; i < 5; i++) {
        for (j = 0; j < SCREEN_WIDTH; j++) {
            put_char_at(j, y + i, ' ', (MAIN_BG << 4) | MAIN_FG);
        }
    }
    
    put_string_at(8, y, description, (MAIN_BG << 4) | MAIN_FG);
    
    put_char_at(x, y + 2, '[', (MAIN_BG << 4) | MAIN_FG);
    
    for (i = 0; i < bars; i++) {
        put_char_at(x + 1 + i, y + 2, 219, (BLUE << 4) | WHITE);
    }
    
    for (i = bars; i < barWidth; i++) {
        put_char_at(x + 1 + i, y + 2, 176, (LIGHTGRAY << 4) | BLACK);
    }
    
    put_char_at(x + barWidth + 1, y + 2, ']', (MAIN_BG << 4) | MAIN_FG);
    
    sprintf(percentStr, " %3d%%", percent);
    put_string_at(x + barWidth + 2, y + 2, percentStr, (MAIN_BG << 4) | MAIN_FG);
}

int display_menu(int x, int y, int width, int height, const char *title, char **items, int item_count) {
    int key = 0;
    int selected = 0;
    int redraw = 1;
    int i, j;
    int item_x, item_y, len;
    int result = -1;
    
    if (x < 0) x = 0;
    if (y < 0) y = 0;
    if (x + width > SCREEN_WIDTH) width = SCREEN_WIDTH - x;
    if (y + height > SCREEN_HEIGHT) height = SCREEN_HEIGHT - y;
    if (width <= 0 || height <= 0) return -1;
    
    while (1) {
        if (redraw) {
            for (i = y; i < y + height; i++) {
                for (j = 0; j < SCREEN_WIDTH; j++) {
                    put_char_at(j, i, ' ', (MAIN_BG << 4) | MAIN_FG);
                }
            }
            
            draw_box(x, y, width, height);
            
            put_string_at((SCREEN_WIDTH - strlen(title)) / 2, y, title, (WHITE << 4) | BLACK);
            
            for (i = 0; i < item_count; i++) {
                item_x = x + 2;
                item_y = y + 2 + i;
                
                if (i == selected) {
                    put_string_at(item_x, item_y, "> ", (HIGHLIGHT_BG << 4) | HIGHLIGHT_FG);
                    put_string_at(item_x + 2, item_y, items[i], (HIGHLIGHT_BG << 4) | HIGHLIGHT_FG);
                    
                    len = strlen(items[i]) + 2;
                    for (j = len; j < width - 4; j++) {
                        put_char_at(item_x + j, item_y, ' ', (HIGHLIGHT_BG << 4) | HIGHLIGHT_FG);
                    }
                } else {
                    put_string_at(item_x, item_y, "  ", (WHITE << 4) | BLACK);
                    put_string_at(item_x + 2, item_y, items[i], (WHITE << 4) | BLACK);
                }
            }
            
            redraw = 0;
        }
        
        if (kbhit()) {
            key = getch();
            
            if (key == 0) {
                key = getch();
                
                if (key == 72) {
                    if (selected > 0) {
                        selected--;
                        redraw = 1;
                    }
                } else if (key == 80) {
                    if (selected < item_count - 1) {
                        selected++;
                        redraw = 1;
                    }
                } else if (key == 0x3B) {
                    display_help("menu");
                    redraw = 1;
                }
            } else if (key == 13) {
                result = selected;
                break;
            } else if (key == 27) {
                result = -1;
                break;
            } else if (key == 0x3D) {
                if (display_exit_confirmation()) {
                    result = -2;
                    break;
                }
                redraw = 1;
            }
        }
        
        delay(50);
    }
    
    return result;
}

void display_help(const char* topic) {
    display_help_screens(topic);
}

int prompt_for_disk(char disk) {
    char message[256];
    int width = 60;
    int height = 8;
    int x = (SCREEN_WIDTH - width) / 2;
    int y = (SCREEN_HEIGHT - height) / 2;
    char* insert_disk = "Please insert the";
    char* into_drive = "into drive";
    char* press_enter = "and press ENTER.";
    char label_str[50];
    
    clear_screen();
    draw_main_content_area();
    
    snprintf(message, sizeof(message), "%s %c %s '%s' %s", 
             insert_disk, disk, into_drive, get_disk_label(disk), press_enter);
    
    draw_box(x, y, width, height);
    
    put_string_at(x + 2, y + 2, message, (WHITE << 4) | BLACK);
    
    put_string_at(x + 2, y + 4, "Disk label:", (WHITE << 4) | BLACK);
    
    snprintf(label_str, sizeof(label_str), "%c: %s", disk, get_disk_label(disk));
    put_string_at(x + 2, y + 5, label_str, (WHITE << 4) | BLACK);
    
    while (1) {
        if (kbhit()) {
            int key = getch();
            if (key == 13) return 1;
            if (key == 27) {
                if (display_exit_confirmation()) {
                    return 0;
                }
            }
            if (key == 0) {
                key = getch();
                if (key == 0x3B) {
                    display_help("disk");
                } else if (key == 0x3D) {
                    if (display_exit_confirmation()) {
                        return 0;
                    }
                }
            }
        }
        delay(50);
    }
}
