#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <conio.h>
#include <dos.h>
#include "screens.h"
#include "ui.h"
#include "resources.h"
#include "help.h"
#include "dialog.h"

void setup_loading_screen(void) {
    int i;
    clear_screen_direct();
    draw_standard_header();
    
    /* Только сообщение в футере */
    for (i = 0; i < SCREEN_WIDTH; i++) {
        put_char_at(i, SCREEN_HEIGHT - 1, ' ', (HEADER_BG << 4) | HEADER_FG);
    }
    put_string_at(2, SCREEN_HEIGHT - 1, "Reading SETUP.INF...", (HEADER_BG << 4) | HEADER_FG);
}

void setup_welcome(void) {
    int key;
    int done = 0;
    
    clear_screen();
    draw_main_content_area();
    draw_standard_header();
    
    put_string_at(8, 4, "Welcome to Setup.", (MAIN_BG << 4) | WHITE);
    put_string_at(8, 6, "The Setup program for Windows 3.0 prepares Windows", (MAIN_BG << 4) | MAIN_FG);
    put_string_at(8, 7, "to run on your computer.", (MAIN_BG << 4) | MAIN_FG);
    put_string_at(8, 8, "Each Setup screen has basic instructions for", (MAIN_BG << 4) | MAIN_FG);
    put_string_at(8, 9, "completing a step of the installation.", (MAIN_BG << 4) | MAIN_FG);
    put_string_at(8, 10, "If you want additional information and instructions", (MAIN_BG << 4) | MAIN_FG);
    put_string_at(8, 11, "about a screen or option, please press the Help key, F1.", (MAIN_BG << 4) | MAIN_FG);
    put_string_at(8, 13, "To learn how to use Windows Setup, press F1.", (MAIN_BG << 4) | MAIN_FG);
    put_string_at(8, 15, "To install Windows on your computer now, press ENTER.", (MAIN_BG << 4) | MAIN_FG);
    put_string_at(8, 17, "To exit Setup without installing Windows, press F3.", (MAIN_BG << 4) | MAIN_FG);

    while (!done) {
        if (kbhit()) {
            key = getch();
            if (key == 13) {
                done = 1;
            } else if (key == 0) {
                key = getch();
                if (key == 0x3B) {
                    display_help("welcome");
                } else if (key == 0x3D) {
                    if (display_exit_confirmation()) {
                        exit(0);
                    }
                }
            } else if (key == 27) {
                if (display_exit_confirmation()) {
                    exit(0);
                }
            }
        }
        delay(50);
    }
}

int setup_type_menu(void) {
    return 1;
}

void setup_directory_menu(char *directory) {
    char input[80];
    char* default_dir = "C:\\WINDOWS";
    int pos;
    int key;
    int done = 0;
    int i;
    
    strcpy(input, default_dir);
    
    clear_screen();
    draw_main_content_area();
    draw_standard_header();
    
    put_string_at(8, 5, "Setup is ready to install Windows into the following", (MAIN_BG << 4) | MAIN_FG);
    put_string_at(8, 6, "directory, which it will create on your hard disk:", (MAIN_BG << 4) | MAIN_FG);
    put_string_at(8, 8, "Windows directory:", (MAIN_BG << 4) | MAIN_FG);
    
    for (i = 0; i < 40; i++) {
        put_char_at(25 + i, 9, ' ', (INPUT_BG << 4) | INPUT_FG);
    }
    
    put_string_at(25, 9, input, (INPUT_BG << 4) | INPUT_FG);
    
    put_string_at(8, 12, "If you want to install Windows in a different directory", (MAIN_BG << 4) | MAIN_FG);
    put_string_at(8, 13, "and/or drive, use the BACKSPACE key to erase the name", (MAIN_BG << 4) | MAIN_FG);
    put_string_at(8, 14, "shown above. Then type the name of the directory where", (MAIN_BG << 4) | MAIN_FG);
    put_string_at(8, 15, "you want to store your Windows files.", (MAIN_BG << 4) | MAIN_FG);
    put_string_at(8, 17, "When the directory shown above is correctly named,", (MAIN_BG << 4) | MAIN_FG);
    put_string_at(8, 18, "press ENTER to continue Setup.", (MAIN_BG << 4) | MAIN_FG);
    
    pos = strlen(input);
    set_cursor_pos(25 + pos, 9);
    show_cursor();
    
    while (!done) {
        if (kbhit()) {
            key = getch();
            
            if (key == 13) {
                strcpy(directory, input);
                done = 1;
            } else if (key == 27) {
                if (display_exit_confirmation()) {
                    exit(0);
                }
            } else if (key == 8) {
                if (pos > 0) {
                    pos--;
                    input[pos] = '\0';
                    put_char_at(25 + pos, 9, ' ', (INPUT_BG << 4) | INPUT_FG);
                    set_cursor_pos(25 + pos, 9);
                }
            } else if (key >= 32 && key <= 126) {
                if (pos < 79) {
                    input[pos] = key;
                    pos++;
                    input[pos] = '\0';
                    put_char_at(25 + pos - 1, 9, key, (INPUT_BG << 4) | INPUT_FG);
                    set_cursor_pos(25 + pos, 9);
                }
            } else if (key == 0) {
                key = getch();
                if (key == 0x3B) {
                    display_help("directory");
                } else if (key == 0x3D) {
                    if (display_exit_confirmation()) {
                        exit(0);
                    }
                }
            }
        }
        delay(50);
    }
    
    hide_cursor();
}

void display_configuration_confirmation(const char* windows_dir, const char* setup_type) {
    int key;
    int done = 0;
    
    clear_screen();
    draw_main_content_area();
    draw_standard_header();
    
    put_string_at(8, 5, "Setup is ready to install Windows 3.0.", (MAIN_BG << 4) | MAIN_FG);
    put_string_at(8, 7, "The following settings will be used:", (MAIN_BG << 4) | MAIN_FG);
    put_string_at(8, 9, "Installation Directory:", (MAIN_BG << 4) | MAIN_FG);
    put_string_at(8, 10, windows_dir, (MAIN_BG << 4) | MAIN_FG);
    put_string_at(8, 12, "Setup Type: ", (MAIN_BG << 4) | MAIN_FG);
    put_string_at(20, 12, setup_type, (MAIN_BG << 4) | MAIN_FG);
    put_string_at(8, 14, "Press ENTER to continue Setup.", (MAIN_BG << 4) | MAIN_FG);
    
    while (!done) {
        if (kbhit()) {
            key = getch();
            if (key == 13) {
                done = 1;
            } else if (key == 0) {
                key = getch();
                if (key == 0x3B) {  /* F1 - Help */
                    display_help("configuration");
                    /* После возврата из help перерисовываем экран */
                    clear_screen();
                    draw_main_content_area();
                    
                    put_string_at(8, 5, "Setup is ready to install Windows 3.0.", (MAIN_BG << 4) | MAIN_FG);
                    put_string_at(8, 7, "The following settings will be used:", (MAIN_BG << 4) | MAIN_FG);
                    put_string_at(8, 9, "Installation Directory:", (MAIN_BG << 4) | MAIN_FG);
                    put_string_at(8, 10, windows_dir, (MAIN_BG << 4) | MAIN_FG);
                    put_string_at(8, 12, "Setup Type: ", (MAIN_BG << 4) | MAIN_FG);
                    put_string_at(20, 12, setup_type, (MAIN_BG << 4) | MAIN_FG);
                    put_string_at(8, 14, "Press ENTER to continue Setup.", (MAIN_BG << 4) | MAIN_FG);
                } else if (key == 0x3D) {  /* F3 - Exit */
                    if (display_yesno("Are you sure you want to exit Setup?")) {
                        exit(0);
                    }
                    /* После возврата из yesno перерисовываем экран */
                    clear_screen();
                    draw_main_content_area();
                    
                    put_string_at(8, 5, "Setup is ready to install Windows 3.0.", (MAIN_BG << 4) | MAIN_FG);
                    put_string_at(8, 7, "The following settings will be used:", (MAIN_BG << 4) | MAIN_FG);
                    put_string_at(8, 9, "Installation Directory:", (MAIN_BG << 4) | MAIN_FG);
                    put_string_at(8, 10, windows_dir, (MAIN_BG << 4) | MAIN_FG);
                    put_string_at(8, 12, "Setup Type: ", (MAIN_BG << 4) | MAIN_FG);
                    put_string_at(20, 12, setup_type, (MAIN_BG << 4) | MAIN_FG);
                    put_string_at(8, 14, "Press ENTER to continue Setup.", (MAIN_BG << 4) | MAIN_FG);
                }
            } else if (key == 27) {
                if (display_yesno("Are you sure you want to exit Setup?")) {
                    exit(0);
                }
                /* После возврата из yesno перерисовываем экран */
                clear_screen();
                draw_main_content_area();
                
                put_string_at(8, 5, "Setup is ready to install Windows 3.0.", (MAIN_BG << 4) | MAIN_FG);
                put_string_at(8, 7, "The following settings will be used:", (MAIN_BG << 4) | MAIN_FG);
                put_string_at(8, 9, "Installation Directory:", (MAIN_BG << 4) | MAIN_FG);
                put_string_at(8, 10, windows_dir, (MAIN_BG << 4) | MAIN_FG);
                put_string_at(8, 12, "Setup Type: ", (MAIN_BG << 4) | MAIN_FG);
                put_string_at(20, 12, setup_type, (MAIN_BG << 4) | MAIN_FG);
                put_string_at(8, 14, "Press ENTER to continue Setup.", (MAIN_BG << 4) | MAIN_FG);
            }
        }
        delay(50);
    }
}

void display_completion_screen(void) {
    int i;
    clear_screen();
    draw_main_content_area();
    draw_standard_header();
    
    put_string_at(8, 10, "Setup is complete.", (MAIN_BG << 4) | MAIN_FG);
    put_string_at(8, 12, "Please remove any disks from your drives and", (MAIN_BG << 4) | MAIN_FG);
    put_string_at(8, 13, "press ENTER to start Windows and continue Setup.", (MAIN_BG << 4) | MAIN_FG);
    
    /* Специальный футер для экрана завершения */
    for (i = 0; i < SCREEN_WIDTH; i++) {
        put_char_at(i, SCREEN_HEIGHT - 1, ' ', (HEADER_BG << 4) | HEADER_FG);
    }
    put_string_at(2, SCREEN_HEIGHT - 1, "ENTER=Continue", (HEADER_BG << 4) | HEADER_FG);
    
    while (1) {
        if (kbhit()) {
            int key = getch();
            if (key == 13) {
                break;
            } else if (key == 0) {
                key = getch();
                if (key == 0x3B) {
                    display_help("completion");
                } else if (key == 0x3D) {
                    if (display_exit_confirmation()) {
                        exit(0);
                    }
                }
            } else if (key == 27) {
                if (display_exit_confirmation()) {
                    exit(0);
                }
            }
        }
        delay(50);
    }
}
