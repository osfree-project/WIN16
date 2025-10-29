#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <conio.h>
#include <dos.h>
#include <malloc.h>
#include "dialog.h"
#include "ui.h"
#include "video.h"

int display_yesno(const char *question) {
    char *items[] = {"Yes", "No"};
    int result;
    struct ScreenChar far *save_buffer;
    
    /* Save the area where menu will be displayed */
    save_buffer = (struct ScreenChar far *)_fmalloc(40 * 8 * sizeof(struct ScreenChar));
    if (save_buffer != NULL) {
        save_screen_area(20, 10, 40, 8, save_buffer);
    }
    
    result = display_menu(20, 10, 40, 8, question, items, 2);
    
    /* Restore the saved screen area */
    if (save_buffer != NULL) {
        restore_screen_area(20, 10, 40, 8, save_buffer);
        _ffree(save_buffer);
    }
    
    if (result == -2) {
        exit(0);
    }
    
    return (result == 0);
}

void display_message(const char *message) {
    int width;
    int height;
    int x;
    int y;
    struct ScreenChar far *save_buffer;
    int key;
    
    width = strlen(message) + 4;
    height = 5;
    x = (SCREEN_WIDTH - width) / 2;
    y = (SCREEN_HEIGHT - height) / 2;
    
    if (width < 20) width = 20;
    if (width > 76) width = 76;
    
    /* Save the area where message box will be displayed */
    save_buffer = (struct ScreenChar far *)_fmalloc(width * height * sizeof(struct ScreenChar));
    if (save_buffer != NULL) {
        save_screen_area(x, y, width, height, save_buffer);
    }
    
    draw_box(x, y, width, height);
    
    put_string_at(x + 2, y + 2, message, (WHITE << 4) | BLACK);
    
    put_string_at(x + (width - 2) / 2, y + height - 2, "OK", (HIGHLIGHT_BG << 4) | HIGHLIGHT_FG);
    
    while (1) {
        if (kbhit()) {
            key = getch();
            if (key == 13 || key == 27) {
                break;
            } else if (key == 0) {
                key = getch();
                if (key == 0x3B) {
                    display_help("message");
                }
            }
        }
        delay(50);
    }
    
    /* Restore the saved screen area */
    if (save_buffer != NULL) {
        restore_screen_area(x, y, width, height, save_buffer);
        _ffree(save_buffer);
    }
}

int display_file_error(const char* filename) {
    char message[256];
    int width = 60;
    int height = 8;
    int x = (SCREEN_WIDTH - width) / 2;
    int y = (SCREEN_HEIGHT - height) / 2;
    char* file_not_found = "File not found:";
    char* retry_cancel = "Press R to Retry or C to Cancel";
    int result = 0;
    struct ScreenChar far *save_buffer;
    int key;
    
    /* Save the area where dialog will be displayed */
    save_buffer = (struct ScreenChar far *)_fmalloc(width * height * sizeof(struct ScreenChar));
    if (save_buffer != NULL) {
        save_screen_area(x, y, width, height, save_buffer);
    }
    
    draw_box(x, y, width, height);
    
    put_string_at(x + 2, y + 2, file_not_found, (WHITE << 4) | BLACK);
    put_string_at(x + 2, y + 3, filename, (WHITE << 4) | BLACK);
    put_string_at(x + 2, y + 5, retry_cancel, (WHITE << 4) | BLACK);
    
    while (1) {
        if (kbhit()) {
            key = tolower(getch());
            if (key == 'r') {
                result = 1;
                break;
            }
            if (key == 'c') {
                result = 0;
                break;
            }
            if (key == 27) {
                result = 0;
                break;
            }
            if (key == 0) {
                key = getch();
                if (key == 0x3B) {
                    display_help("file_error");
                } else if (key == 0x3D) {
                    if (display_exit_confirmation()) {
                        result = 0;
                        break;
                    }
                }
            }
        }
        delay(50);
    }
    
    /* Restore the saved screen area */
    if (save_buffer != NULL) {
        restore_screen_area(x, y, width, height, save_buffer);
        _ffree(save_buffer);
    }
    
    return result;
}

int display_error_dialog(const char* line1, const char* line2, const char* line3) {
    int i, j;
    int dlg_width = 50;
    int dlg_height = 7;
    int dlg_x = (SCREEN_WIDTH - dlg_width) / 2;
    int dlg_y = (SCREEN_HEIGHT - dlg_height) / 3;
    int line_y;
    struct ScreenChar far *save_buffer;
    int key;

    /* Save the area where dialog will be displayed */
    save_buffer = (struct ScreenChar far *)_fmalloc(dlg_width * dlg_height * sizeof(struct ScreenChar));
    if (save_buffer != NULL) {
        save_screen_area(dlg_x, dlg_y, dlg_width, dlg_height, save_buffer);
    }

    /* ДВОЙНАЯ внешняя рамка */
    put_char_at(dlg_x, dlg_y, 201, (LIGHTGRAY << 4) | RED);
    put_char_at(dlg_x + dlg_width - 1, dlg_y, 187, (LIGHTGRAY << 4) | RED);
    put_char_at(dlg_x, dlg_y + dlg_height - 1, 200, (LIGHTGRAY << 4) | RED);
    put_char_at(dlg_x + dlg_width - 1, dlg_y + dlg_height - 1, 188, (LIGHTGRAY << 4) | RED);
    
    for (i = 1; i < dlg_width - 1; i++) {
        put_char_at(dlg_x + i, dlg_y, 205, (LIGHTGRAY << 4) | RED);
        put_char_at(dlg_x + i, dlg_y + dlg_height - 1, 205, (LIGHTGRAY << 4) | RED);
    }
    
    for (i = 1; i < dlg_height - 1; i++) {
        put_char_at(dlg_x, dlg_y + i, 186, (LIGHTGRAY << 4) | RED);
        put_char_at(dlg_x + dlg_width - 1, dlg_y + i, 186, (LIGHTGRAY << 4) | RED);
    }
    
    /* ОДИНАРНАЯ горизонтальная линия для разделения F3=Exit */
    line_y = dlg_y + dlg_height - 3;
    put_char_at(dlg_x, line_y, 199, (LIGHTGRAY << 4) | RED);
    put_char_at(dlg_x + dlg_width - 1, line_y, 182, (LIGHTGRAY << 4) | RED);
    for (i = 1; i < dlg_width - 1; i++) {
        put_char_at(dlg_x + i, line_y, 196, (LIGHTGRAY << 4) | RED);
    }

    /* Заливка */
    for (i = 1; i < dlg_height - 1; i++) {
        for (j = 1; j < dlg_width - 1; j++) {
            put_char_at(dlg_x + j, dlg_y + i, ' ', (LIGHTGRAY << 4) | RED);
        }
    }
    
    /* Текст сообщения */
    put_string_at(dlg_x + 2, dlg_y + 1, line1, (LIGHTGRAY << 4) | RED);
    put_string_at(dlg_x + 2, dlg_y + 2, line2, (LIGHTGRAY << 4) | RED);
    put_string_at(dlg_x + 2, dlg_y + 3, line3, (LIGHTGRAY << 4) | RED);
    
    /* Текст F3=Exit под разделительной линией */
    put_string_at(dlg_x + 3, line_y + 1, "F3=Exit", (LIGHTGRAY << 4) | RED);
    
    /* Ждем нажатия F3 для выхода */
    while (1) {
        if (kbhit()) {
            key = getch();
            if (key == 0) {
                key = getch();
                if (key == 0x3D) { /* F3 */
                    break;
                }
            }
        }
        delay(50);
    }
    
    /* Restore the saved screen area */
    if (save_buffer != NULL) {
        restore_screen_area(dlg_x, dlg_y, dlg_width, dlg_height, save_buffer);
        _ffree(save_buffer);
    }
    
    return 1;
}

int display_exit_confirmation(void) {
    int i, j;
    int dlg_width = 60;
    int dlg_height = 8;
    int dlg_x = (SCREEN_WIDTH - dlg_width) / 2;
    int dlg_y = (SCREEN_HEIGHT - dlg_height) / 3;
    int line_y;
    int result = 0;
    struct ScreenChar far *save_buffer;
    int key;

    /* Save the area where dialog will be displayed */
    save_buffer = (struct ScreenChar far *)_fmalloc(dlg_width * dlg_height * sizeof(struct ScreenChar));
    if (save_buffer != NULL) {
        save_screen_area(dlg_x, dlg_y, dlg_width, dlg_height, save_buffer);
    }

    /* ДВОЙНАЯ внешняя рамка - БЕЛЫЙ ФОН, КРАСНЫЙ ТЕКСТ */
    put_char_at(dlg_x, dlg_y, 201, (LIGHTGRAY << 4) | RED);
    put_char_at(dlg_x + dlg_width - 1, dlg_y, 187, (LIGHTGRAY << 4) | RED);
    put_char_at(dlg_x, dlg_y + dlg_height - 1, 200, (LIGHTGRAY << 4) | RED);
    put_char_at(dlg_x + dlg_width - 1, dlg_y + dlg_height - 1, 188, (LIGHTGRAY << 4) | RED);
    
    for (i = 1; i < dlg_width - 1; i++) {
        put_char_at(dlg_x + i, dlg_y, 205, (LIGHTGRAY << 4) | RED);
        put_char_at(dlg_x + i, dlg_y + dlg_height - 1, 205, (LIGHTGRAY << 4) | RED);
    }
    
    for (i = 1; i < dlg_height - 1; i++) {
        put_char_at(dlg_x, dlg_y + i, 186, (LIGHTGRAY << 4) | RED);
        put_char_at(dlg_x + dlg_width - 1, dlg_y + i, 186, (LIGHTGRAY << 4) | RED);
    }

    /* Заливка белым фоном */
    for (i = 1; i < dlg_height - 1; i++) {
        for (j = 1; j < dlg_width - 1; j++) {
            put_char_at(dlg_x + j, dlg_y + i, ' ', (LIGHTGRAY << 4) | RED);
        }
    }
    
    /* Текст сообщения  */
    put_string_at(dlg_x + 2, dlg_y + 1, "Windows is not installed properly on your", (LIGHTGRAY << 4) | RED);
    put_string_at(dlg_x + 2, dlg_y + 2, "system. If you exit Setup now, you will have", (LIGHTGRAY << 4) | RED);
    put_string_at(dlg_x + 2, dlg_y + 3, "to run Setup again to install Windows.", (LIGHTGRAY << 4) | RED);
    
    /* ОДИНАРНАЯ горизонтальная линия для разделения F3=Exit */
    line_y = dlg_y + dlg_height - 3;
    put_char_at(dlg_x, line_y, 199, (LIGHTGRAY << 4) | RED);
    put_char_at(dlg_x + dlg_width - 1, line_y, 182, (LIGHTGRAY << 4) | RED);
    for (i = 1; i < dlg_width - 1; i++) {
        put_char_at(dlg_x + i, line_y, 196, (LIGHTGRAY << 4) | RED);
    }

    /* Инструкции внизу  */
    put_string_at(dlg_x + 3, dlg_y + 6, "F3=Exit  ENTER=Continue", (LIGHTGRAY << 4) | RED);
    
    /* Ждем нажатия ENTER или F3 */
    while (1) {
        if (kbhit()) {
            key = getch();
            if (key == 13) { /* ENTER */
                result = 0;
                break;
            } else if (key == 0) {
                key = getch();
                if (key == 0x3D) { /* F3 */
                    result = 1;
                    break;
                }
            }
        }
        delay(50);
    }
    
    /* Restore the saved screen area */
    if (save_buffer != NULL) {
        restore_screen_area(dlg_x, dlg_y, dlg_width, dlg_height, save_buffer);
        _ffree(save_buffer);
    }
    
    return result;
}
