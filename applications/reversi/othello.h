#ifndef OTHELLO_H
#define OTHELLO_H

/* #define GERMAN */

#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define OTHELLO_MAX_PATH 260
#define MAKEWPARAM(lo, hi) ((WPARAM)MAKELONG(lo, hi))

#define BST_CHECKED         1
#define BST_UNCHECKED       0
#define DID_OK              IDOK

#define WD_MAIN            255
#define DB_PRODINFO        257
#define DB_EVE             12000
#define PB_HELP            258

#define ICO_MAIN           276
#define TI_LOGO            277

#define SM_GAME            259
#define MI_NEW             260
#define MI_HINT            261

#define SM_OPTIONS         262
#define MI_BEGINNER        263
#define MI_ADVANCED        264
#define MI_MASTER          265
#define MI_PLAYER          266
#define MI_COMPUTER        267
#define MI_SOUND           268

#define SM_HELP            269
#define MI_HELPPRODUCTINFO 274

#define SM_BGCOLOR      15100
#define MI_COLOR0       15100
#define MI_COLOR1       15101
#define MI_COLOR2       15102
#define MI_COLOR3       15103
#define MI_COLOR4       15104
#define MI_COLOR5       15105
#define MI_COLOR6       15106
#define MI_COLOR7       15107
#define MI_COLOR8       15108
#define MI_COLOR9       15109
#define MI_COLOR10      15110
#define MI_COLOR11      15111
#define MI_COLOR12      15112
#define MI_COLOR13      15113
#define MI_COLOR14      15114
#define MI_COLOR15      15115

#define DIVISIONS     8
#define RETURN_ERROR  1
#define EMPTY         0
#define PLAYER        1
#define COMPUTER      5
#define IND           32767

#define CB_EVESOUNDS    12010
#define LB_EVE          12020
#define LB_EVESOUNDS    12030
#define SL_EVESOUNDS    12040
#define PB_PLAYEVE      12050

typedef struct {
    short sEvent;
    char  chWavFile[OTHELLO_MAX_PATH];
} SOUNDEVENT;
typedef SOUNDEVENT *PSOUNDEVENT;

typedef struct _BOARD {
    short sField[8][8];
} BOARD;
typedef BOARD *PBOARD;

int PASCAL WinMain(HINSTANCE, HINSTANCE, LPSTR, int);
LRESULT WINAPI WndProc(HWND, UINT, WPARAM, LPARAM);
BOOL WINAPI dpProdInfo(HWND, UINT, WPARAM, LPARAM);
BOOL WINAPI dpEve(HWND, UINT, WPARAM, LPARAM);

void Error(BOOL);
void SetPointer(short, short);
void DisplayMessage(const char *);
void Result(BOARD, short *, short *);
void DoComputerMove(HWND, BOOL);
void vdGetWavFile(unsigned short, char *);
void vdPlayWavFileAsyncEve(PSOUNDEVENT);

void CopyBoard(PBOARD, PBOARD);
BOOL fIsMovePossible(BOARD, short, short, short);
short MakeMove(PBOARD, short, short, short, BOOL, BOOL, short, short, HWND);
short sOthello(BOARD, short, short, short, short, short, short *, short *, BOOL *, short);
BOOL fMustPass(BOARD, short);
BOOL fGameOver(BOARD);
short sFlipped(PBOARD, short, short, short);
short sSquare(BOARD, short, short, short, short);
short sBewertung(BOARD, short, short, short);

#endif
