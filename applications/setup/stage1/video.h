#ifndef VIDEO_H
#define VIDEO_H

#define BLACK 0
#define BLUE 1
#define GREEN 2
#define CYAN 3
#define RED 4
#define MAGENTA 5
#define BROWN 6
#define LIGHTGRAY 7
#define DARKGRAY 8
#define LIGHTBLUE 9
#define LIGHTGREEN 10
#define LIGHTCYAN 11
#define LIGHTRED 12
#define LIGHTMAGENTA 13
#define YELLOW 14
#define WHITE 15

#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 25

struct ScreenChar {
    unsigned char character;
    unsigned char attribute;
};

/* Low-level video functions */
void set_video_mode(int mode);
void set_cursor_pos(int x, int y);
int get_cursor_x(void);
int get_cursor_y(void);
void hide_cursor(void);
void show_cursor(void);
void put_char_at(int x, int y, char c, int color);
void put_string_at(int x, int y, const char *str, int color);
void save_screen_state(void);
void restore_screen_state(void);
void clear_screen_direct(void);

/* New functions for saving/restoring screen areas */
void save_screen_area(int x, int y, int width, int height, struct ScreenChar far *buffer);
void restore_screen_area(int x, int y, int width, int height, struct ScreenChar far *buffer);

#endif
