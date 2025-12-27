#include <stdio.h>
#include <string.h>
#include <conio.h>
#include "ui.h"
#include "help.h"
#include "video.h"

#define MAX_HELP_SCREENS 6
#define MAX_LINES 21  /* Увеличено до 21 для 6-го экрана */
#define MAX_LENGTH 74  /* 80 - 4 (отступ) - 2 (запас) */

typedef struct {
    char text[MAX_LENGTH];
    unsigned char attribute;
} HelpLine;

typedef struct {
    int line_count;
    HelpLine lines[MAX_LINES];
} HelpScreen;

static const HelpScreen welcome_help_screens[MAX_HELP_SCREENS];
static const int welcome_screen_count = 6;

static const HelpScreen directory_help_screen;
static const HelpScreen configuration_help_screen;

/* Инициализация welcome_help_screens */
static const HelpScreen welcome_help_screens[MAX_HELP_SCREENS] = {
    /* Screen 1: Microsoft Windows Setup 3.0 */
    {
        13,
        {
            {"Microsoft Windows Setup 3.0", (HELP_HEADER_BG << 4) | HELP_HEADER_FG},
            {"", (HELP_BG << 4) | HELP_FG},
            {"The Windows Setup program makes it easy for you to install", (HELP_BG << 4) | HELP_FG},
            {"Windows on your computer. Setup determines what kind of", (HELP_BG << 4) | HELP_FG},
            {"computer system you are using and presents appropriate", (HELP_BG << 4) | HELP_FG},
            {"options for you to choose from during installation.", (HELP_BG << 4) | HELP_FG},
            {"", (HELP_BG << 4) | HELP_FG},
            {"To accept Setup's choices all you have to do is press ENTER.", (HELP_BG << 4) | HELP_FG},
            {"If you want to change a recommended setting, you simply", (HELP_BG << 4) | HELP_FG},
            {"select the item you want to change and choose a different", (HELP_BG << 4) | HELP_FG},
            {"setting. If you need more information to decide on a certain", (HELP_BG << 4) | HELP_FG},
            {"option, you can always get Help by pressing F1 as you did", (HELP_BG << 4) | HELP_FG},
            {"just now.", (HELP_BG << 4) | HELP_FG}
        }
    },
    /* Screen 2: The Setup Program */
    {
        15,
        {
            {"The Setup Program", (HELP_HEADER_BG << 4) | HELP_HEADER_FG},
            {"", (HELP_BG << 4) | HELP_FG},
            {"The Setup program has two parts: a DOS portion and a Windows", (HELP_BG << 4) | HELP_FG},
            {"portion. Right now, you are in the DOS portion of Setup.", (HELP_BG << 4) | HELP_FG},
            {"While in DOS, Setup accomplishes three things:", (HELP_BG << 4) | HELP_FG},
            {"", (HELP_BG << 4) | HELP_FG},
            {"* It identifies the type of computer, display, mouse, keyboard,", (HELP_BG << 4) | HELP_FG},
            {"  keyboard layout, language, and network you have.", (HELP_BG << 4) | HELP_FG},
            {"* It confirms the directory where you will store the Windows files.", (HELP_BG << 4) | HELP_FG},
            {"* It copies essential files to your hard disk so it can start Windows.", (HELP_BG << 4) | HELP_FG},
            {"", (HELP_BG << 4) | HELP_FG},
            {"Setup gives you a chance to verify this information and change", (HELP_BG << 4) | HELP_FG},
            {"specific settings before showing the next screen. After", (HELP_BG << 4) | HELP_FG},
            {"completing the DOS portion, Setup starts Windows so that it", (HELP_BG << 4) | HELP_FG},
            {"can install additional options and complete the installation.", (HELP_BG << 4) | HELP_FG}
        }
    },
    /* Screen 3: What You Should Know about Your System */
    {
        16,
        {
            {"What You Should Know about Your System", (HELP_HEADER_BG << 4) | HELP_HEADER_FG},
            {"", (HELP_BG << 4) | HELP_FG},
            {"Setup does its best to determine the appropriate settings for", (HELP_BG << 4) | HELP_FG},
            {"you during installation, but it helps if you also know the", (HELP_BG << 4) | HELP_FG},
            {"following things about your system:", (HELP_BG << 4) | HELP_FG},
            {"", (HELP_BG << 4) | HELP_FG},
            {"* what kind of computer (IBM or compatible) you have.", (HELP_BG << 4) | HELP_FG},
            {"* what kind of graphics adapter (EGA or VGA) you have.", (HELP_BG << 4) | HELP_FG},
            {"* what kind of mouse (Microsoft or other) you have.", (HELP_BG << 4) | HELP_FG},
            {"* what kind of printer(s) you will be using.", (HELP_BG << 4) | HELP_FG},
            {"* what kind of keyboard you will be using.", (HELP_BG << 4) | HELP_FG},
            {"* what kind of network your computer is connected to.", (HELP_BG << 4) | HELP_FG},
            {"* which port your printer is connected to.", (HELP_BG << 4) | HELP_FG},
            {"", (HELP_BG << 4) | HELP_FG},
            {"If you are not sure about any of these items, you can press", (HELP_BG << 4) | HELP_FG},
            {"ENTER to accept Setup's recommendations.", (HELP_BG << 4) | HELP_FG}
        }
    },
    /* Screen 4: Setup Keys */
    {
        17,
        {
            {"Setup Keys", (HELP_HEADER_BG << 4) | HELP_HEADER_FG},
            {"", (HELP_BG << 4) | HELP_FG},
            {"While in Setup, you need to use only a few keys to move from", (HELP_BG << 4) | HELP_FG},
            {"screen to screen and select options. A summary of the keys", (HELP_BG << 4) | HELP_FG},
            {"you will use in Setup appears below:", (HELP_BG << 4) | HELP_FG},
            {"", (HELP_BG << 4) | HELP_FG},
            {"Press this key          To do this", (HELP_BG << 4) | YELLOW},
            {"UP/DOWN ARROW    Move the highlight to the next item in a list.", (HELP_BG << 4) | HELP_FG},
            {"ENTER            Choose a selected option or continue to the", (HELP_BG << 4) | HELP_FG},
            {"                 next Setup screen.", (HELP_BG << 4) | HELP_FG},
            {"F1               Display Help for the current Setup screen.", (HELP_BG << 4) | HELP_FG},
            {"F3               Exit Setup from anywhere in the program.", (HELP_BG << 4) | HELP_FG},
            {"ESC              Return from Help to the Setup screen you left", (HELP_BG << 4) | HELP_FG},
            {"                 or cancel an option.", (HELP_BG << 4) | HELP_FG},
            {"", (HELP_BG << 4) | HELP_FG},
            {"The ENTER, ESC, F1, and F3 keys can be used whenever they", (HELP_BG << 4) | HELP_FG},
            {"appear at the bottom of the screen.", (HELP_BG << 4) | HELP_FG}
        }
    },
    /* Screen 5: Exiting Setup */
    {
        8,
        {
            {"Exiting Setup", (HELP_HEADER_BG << 4) | HELP_HEADER_FG},
            {"", (HELP_BG << 4) | HELP_FG},
            {"To set up Windows properly, you should complete both the DOS", (HELP_BG << 4) | HELP_FG},
            {"and Windows portions of the Setup program. Although it is not", (HELP_BG << 4) | HELP_FG},
            {"recommended, you can exit Setup at any time by pressing F3.", (HELP_BG << 4) | HELP_FG},
            {"Please keep in mind, however, that if you do exit Setup", (HELP_BG << 4) | HELP_FG},
            {"early, Windows will not be installed properly. You will have", (HELP_BG << 4) | HELP_FG},
            {"to run Setup again to install Windows on your computer.", (HELP_BG << 4) | HELP_FG}
        }
    },
    /* Screen 6: Running Setup after Installing Windows */
    {
        21,
        {
            {"Running Setup after Installing Windows", (HELP_HEADER_BG << 4) | HELP_HEADER_FG},
            {"", (HELP_BG << 4) | HELP_FG},
            {"After installing Windows the first time, you can run Setup", (HELP_BG << 4) | HELP_FG},
            {"again whenever you want to change any of the settings you", (HELP_BG << 4) | HELP_FG},
            {"made during your first installation. For example, you might", (HELP_BG << 4) | HELP_FG},
            {"want to tell Windows that you are using a different display,", (HELP_BG << 4) | HELP_FG},
            {"mouse, or keyboard. Or you may have added memory to your", (HELP_BG << 4) | HELP_FG},
            {"computer and want Windows to start using that memory.", (HELP_BG << 4) | HELP_FG},
            {"", (HELP_BG << 4) | HELP_FG},
            {"To run Setup after installing Windows, open the Main group", (HELP_BG << 4) | HELP_FG},
            {"in Program Manager. Then select the Setup icon and choose", (HELP_BG << 4) | HELP_FG},
            {"the Run command from the File menu. Or, type Setup at the", (HELP_BG << 4) | HELP_FG},
            {"DOS prompt and press ENTER to run Setup directly from DOS.", (HELP_BG << 4) | HELP_FG},
            {"", (HELP_BG << 4) | HELP_FG},
            {"If, for some reason, changes to your files leaves Windows", (HELP_BG << 4) | HELP_FG},
            {"inoperable, you may have to reinstall Windows. To do that,", (HELP_BG << 4) | HELP_FG},
            {"exit Windows, delete the Windows directories and files.", (HELP_BG << 4) | HELP_FG},
            {"Then run Setup from the DOS prompt.", (HELP_BG << 4) | HELP_FG},
            {"", (HELP_BG << 4) | HELP_FG},
            {"For more information about using Setup to maintain Windows,", (HELP_BG << 4) | HELP_FG},
            {"see Appendix A in the Windows User's Guide.", (HELP_BG << 4) | HELP_FG}
        }
    }
};

/* Инициализация directory_help_screen */
static const HelpScreen directory_help_screen = {
    13,
    {
        {"Setup needs to know where to put the Windows files it is about", (HELP_BG << 4) | HELP_FG},
        {"to copy to your hard disk. The \"C:\\WINDOWS\" in the edit box", (HELP_BG << 4) | HELP_FG},
        {"means that Setup will put Windows on the C drive of your hard", (HELP_BG << 4) | HELP_FG},
        {"disk in its own directory called \"Windows.\" The drive letter", (HELP_BG << 4) | HELP_FG},
        {"and directory name together create a \"pathname\" that Windows", (HELP_BG << 4) | HELP_FG},
        {"will later use to find system files and programs.", (HELP_BG << 4) | HELP_FG},
        {"", (HELP_BG << 4) | HELP_FG},
        {"You can accept the directory name Setup suggests or type in", (HELP_BG << 4) | HELP_FG},
        {"one of your own. If you enter a directory that is not on drive", (HELP_BG << 4) | HELP_FG},
        {"C or not off the root directory, be sure to include a complete", (HELP_BG << 4) | HELP_FG},
        {"pathname.", (HELP_BG << 4) | HELP_FG},
        {"", (HELP_BG << 4) | HELP_FG},
        {"To return to Setup now, press ESC.", (HELP_BG << 4) | HELP_FG}
    }
};

/* Инициализация configuration_help_screen */
static const HelpScreen configuration_help_screen = {
    20,
    {
        {"It is important that the system information presented on this", (HELP_BG << 4) | HELP_FG},
        {"Setup screen be accurate so that Setup can install Windows", (HELP_BG << 4) | HELP_FG},
        {"correctly on your computer. Setup has done its best to", (HELP_BG << 4) | HELP_FG},
        {"determine the appropriate settings for you. And in most", (HELP_BG << 4) | HELP_FG},
        {"cases, these settings are correct and should not be changed.", (HELP_BG << 4) | HELP_FG},
        {"However, if any of the items are incorrect, or if your", (HELP_BG << 4) | HELP_FG},
        {"computer model or networking software appears on the Hardware", (HELP_BG << 4) | HELP_FG},
        {"Compatibility List with an asterisk beside it, you must", (HELP_BG << 4) | HELP_FG},
        {"change the information Setup has displayed for that item.", (HELP_BG << 4) | HELP_FG},
        {"", (HELP_BG << 4) | HELP_FG},
        {"If you elect to change an item, Setup displays a list of", (HELP_BG << 4) | HELP_FG},
        {"appropriate settings for that item. You select a new setting", (HELP_BG << 4) | HELP_FG},
        {"and then press ENTER. Setup returns to the System Information", (HELP_BG << 4) | HELP_FG},
        {"screen, showing your new setting.", (HELP_BG << 4) | HELP_FG},
        {"", (HELP_BG << 4) | HELP_FG},
        {"When you are satisfied that all the information in the list", (HELP_BG << 4) | HELP_FG},
        {"is correct, select \"No Changes\" and press ENTER to continue", (HELP_BG << 4) | HELP_FG},
        {"with Setup.", (HELP_BG << 4) | HELP_FG},
        {"", (HELP_BG << 4) | HELP_FG},
        {"To return to Setup now, press ESC.", (HELP_BG << 4) | HELP_FG}
    }
};

void display_help_screens(const char* topic)
{
    int current_screen = 0;
    int screen_count = 0;
    const HelpScreen* screens = NULL;
    int key;
    int i;

    /* Save current screen state before showing help */
    save_screen_state();

    /* Determine which help screens to use */
    if (strcmp(topic, "directory") == 0) {
        screens = &directory_help_screen;
        screen_count = 1;
    } else if (strcmp(topic, "configuration") == 0) {
        screens = &configuration_help_screen;
        screen_count = 1;
    } else {
        screens = welcome_help_screens;
        screen_count = welcome_screen_count;
    }

    while (1) {
        /* Clear screen with help background */
        for (i = 0; i < SCREEN_HEIGHT; i++) {
            put_string_at(0, i, "                                                                                ", (HELP_BG << 4) | HELP_FG);
        }

        put_string_at(1, 1, "Setup Help", (HELP_HEADER_BG << 4) | HELP_HEADER_FG);
        put_string_at(0, 2, "\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd", (HELP_HEADER_BG << 4) | HELP_HEADER_FG);

        /* Draw help content with individual line attributes */
        for (i = 0; i < screens[current_screen].line_count && i < SCREEN_HEIGHT - 2; i++) {
            put_string_at(4, 4 + i, 
                        screens[current_screen].lines[i].text, 
                        screens[current_screen].lines[i].attribute);
        }

        /* Draw help footer */
        for (i = 0; i < SCREEN_WIDTH; i++) {
            put_string_at(i, SCREEN_HEIGHT - 1, " ", (HELP_FOOTER_BG << 4) | HELP_FOOTER_FG);
        }
        
        /* Dynamic footer based on current screen position */
        {
            char footer[80];
            if (screen_count == 1) {
                /* Single screen help - only ESC */
                sprintf(footer, "  ESC=Cancel Help");
            } else if (current_screen == 0) {
                /* First screen - no Backspace */
                sprintf(footer, "  ENTER=Continue Reading Help  ESC=Cancel Help");
            } else if (current_screen == screen_count - 1) {
                /* Last screen - no Enter */
                sprintf(footer, "  Backspace=Read Last Help  ESC=Cancel Help");
            } else {
                /* Middle screens - both navigation options */
                sprintf(footer, "  ENTER=Continue Reading Help  Backspace=Read Last Help  ESC=Cancel Help");
            }
            put_string_at(0, SCREEN_HEIGHT - 1, footer, (HELP_FOOTER_BG << 4) | HELP_FOOTER_FG);
        }

        key = getch();
        
        /* Handle key presses */
        if (key == 27) { /* ESC - always exits */
            break;
        } else if (screen_count > 1) {
            /* Multi-screen navigation only for welcome help */
            if (key == 13) { /* ENTER */
                if (current_screen < screen_count - 1) {
                    current_screen++;
                }
                /* On last screen, Enter does nothing */
            } else if (key == 8) { /* Backspace */
                if (current_screen > 0) {
                    current_screen--;
                }
                /* On first screen, Backspace does nothing */
            } else if (key == 0) {
                key = getch(); /* Ignore extended keys */
            }
        } else {
            /* Single screen help - ignore all keys except ESC */
            if (key == 0) {
                key = getch(); /* Ignore extended keys */
            }
        }
    }

    /* Restore original screen state after help is closed */
    restore_screen_state();
}
