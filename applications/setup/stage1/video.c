#include <dos.h>
#include "video.h"

static struct ScreenChar far *video_memory = (struct ScreenChar far *)0xB8000000L;
static struct ScreenChar screen_buffer[SCREEN_HEIGHT][SCREEN_WIDTH];
static int screen_saved = 0;

void set_video_mode(int mode) {
    union REGS regs;
    regs.h.ah = 0x00;
    regs.h.al = mode;
    int86(0x10, &regs, &regs);
}

void set_cursor_pos(int x, int y) {
    union REGS regs;
    regs.h.ah = 0x02;
    regs.h.dh = y;
    regs.h.dl = x;
    regs.h.bh = 0;
    int86(0x10, &regs, &regs);
}

int get_cursor_x(void) {
    union REGS regs;
    regs.h.ah = 0x03;
    regs.h.bh = 0;
    int86(0x10, &regs, &regs);
    return regs.h.dl;
}

int get_cursor_y(void) {
    union REGS regs;
    regs.h.ah = 0x03;
    regs.h.bh = 0;
    int86(0x10, &regs, &regs);
    return regs.h.dh;
}

void hide_cursor(void) {
    union REGS regs;
    regs.h.ah = 0x01;
    regs.h.ch = 0x20;
    regs.h.cl = 0x00;
    int86(0x10, &regs, &regs);
}

void show_cursor(void) {
    union REGS regs;
    regs.h.ah = 0x01;
    regs.h.ch = 0x06;
    regs.h.cl = 0x07;
    int86(0x10, &regs, &regs);
}

void put_char_at(int x, int y, char c, int color) {
    int offset;
    
    if (x < 0 || x >= SCREEN_WIDTH || y < 0 || y >= SCREEN_HEIGHT) {
        return;
    }
    
    offset = y * SCREEN_WIDTH + x;
    video_memory[offset].character = c;
    video_memory[offset].attribute = color;
}

void put_string_at(int x, int y, const char *str, int color) {
    int i;
    for (i = 0; str[i] != '\0' && (x + i) < SCREEN_WIDTH; i++) {
        put_char_at(x + i, y, str[i], color);
    }
}

void save_screen_state(void) {
    int i, j;
    for (i = 0; i < SCREEN_HEIGHT; i++) {
        for (j = 0; j < SCREEN_WIDTH; j++) {
            int offset = i * SCREEN_WIDTH + j;
            screen_buffer[i][j].character = video_memory[offset].character;
            screen_buffer[i][j].attribute = video_memory[offset].attribute;
        }
    }
    screen_saved = 1;
}

void restore_screen_state(void) {
    int i, j;
    if (!screen_saved) return;
    
    for (i = 0; i < SCREEN_HEIGHT; i++) {
        for (j = 0; j < SCREEN_WIDTH; j++) {
            int offset = i * SCREEN_WIDTH + j;
            video_memory[offset].character = screen_buffer[i][j].character;
            video_memory[offset].attribute = screen_buffer[i][j].attribute;
        }
    }
    screen_saved = 0;
}

void clear_screen_direct(void) {
    int i, j;
    for (i = 0; i < SCREEN_HEIGHT; i++) {
        for (j = 0; j < SCREEN_WIDTH; j++) {
            put_char_at(j, i, ' ', (BLUE << 4) | LIGHTGRAY);
        }
    }
    set_cursor_pos(0, 0);
}

/* New function to save a rectangular screen area */
void save_screen_area(int x, int y, int width, int height, struct ScreenChar far *buffer) {
    int i, j;
    int offset;
    
    if (x < 0) x = 0;
    if (y < 0) y = 0;
    if (x + width > SCREEN_WIDTH) width = SCREEN_WIDTH - x;
    if (y + height > SCREEN_HEIGHT) height = SCREEN_HEIGHT - y;
    
    for (i = 0; i < height; i++) {
        for (j = 0; j < width; j++) {
            offset = (y + i) * SCREEN_WIDTH + (x + j);
            buffer[i * width + j] = video_memory[offset];
        }
    }
}

/* New function to restore a rectangular screen area */
void restore_screen_area(int x, int y, int width, int height, struct ScreenChar far *buffer) {
    int i, j;
    int offset;
    
    if (x < 0) x = 0;
    if (y < 0) y = 0;
    if (x + width > SCREEN_WIDTH) width = SCREEN_WIDTH - x;
    if (y + height > SCREEN_HEIGHT) height = SCREEN_HEIGHT - y;
    
    for (i = 0; i < height; i++) {
        for (j = 0; j < width; j++) {
            offset = (y + i) * SCREEN_WIDTH + (x + j);
            video_memory[offset] = buffer[i * width + j];
        }
    }
}
