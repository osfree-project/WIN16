#ifndef MAIN_H
#define MAIN_H

#include <windows.h>
#include "cpl.h"

#define NUM_APPLETS 10

#define COLOR_IDX          0
#define FONTS_IDX          1
#define PORTS_IDX          2
#define MOUSE_IDX          3
#define DESKTOP_IDX        4
#define KEYBOARD_IDX       5
#define PRINTERS_IDX       6
#define INTERNATIONAL_IDX  7
#define DATETIME_IDX       8
#define NETWORK_IDX        9

/* Dialog IDs */
#define DLG_COLOR          200
#define DLG_FONTS          201
#define DLG_PORTS          202
#define DLG_MOUSE          203
#define DLG_DESKTOP        204
#define DLG_KEYBOARD       205
#define DLG_PRINTERS       206
#define DLG_INTERNATIONAL  207
#define DLG_DATETIME       208
#define DLG_NETWORK        209
#define DLG_PORT_SETTINGS  210
#define DLG_COLOR_PALETTE  211

/* Ports */
#define IDC_PRT_LIST        1300
#define IDC_PRT_SETTINGS    1301
#define IDC_PRT_COM_BAUD    1302
#define IDC_PRT_COM_BITS    1303
#define IDC_PRT_COM_PARITY  1304
#define IDC_PRT_COM_STOP    1305
#define IDC_PRT_COM_FLOW    1306

#include "mouse.h"
#include "keyboard.h"
#include "time.h"
#include "fonts.h"
#include "colors.h"
#include "printers.h"
#include "desktop.h"
#include "intl.h"
#include "network.h"

/* Icons (не используются, но оставлены для совместимости) */
#define ICO_COLOR            300
#define ICO_FONTS            301
#define ICO_PORTS            302
#define ICO_MOUSE            303
#define ICO_DESKTOP          304
#define ICO_KEYBOARD         305
#define ICO_PRINTERS         306
#define ICO_INTERNATIONAL    307
#define ICO_DATETIME         308
#define ICO_NETWORK          309

/* Strings */
#define IDS_COLOR_NAME       400
#define IDS_COLOR_INFO       401
#define IDS_FONTS_NAME       402
#define IDS_FONTS_INFO       403
#define IDS_PORTS_NAME       404
#define IDS_PORTS_INFO       405
#define IDS_MOUSE_NAME       406
#define IDS_MOUSE_INFO       407
#define IDS_DESKTOP_NAME     408
#define IDS_DESKTOP_INFO     409
#define IDS_KEYBOARD_NAME    410
#define IDS_KEYBOARD_INFO    411
#define IDS_PRINTERS_NAME    412
#define IDS_PRINTERS_INFO    413
#define IDS_INTERNATIONAL_NAME 414
#define IDS_INTERNATIONAL_INFO 415
#define IDS_DATETIME_NAME    416
#define IDS_DATETIME_INFO    417
#define IDS_NETWORK_NAME     418
#define IDS_NETWORK_INFO     419

extern HINSTANCE g_hInst;

BOOL WINAPI MouseDlgProc(HWND, UINT, WPARAM, LPARAM);
BOOL WINAPI KeyboardDlgProc(HWND, UINT, WPARAM, LPARAM);
BOOL WINAPI DateTimeDlgProc(HWND, UINT, WPARAM, LPARAM);
BOOL WINAPI DesktopDlgProc(HWND, UINT, WPARAM, LPARAM);
BOOL WINAPI InternationalDlgProc(HWND, UINT, WPARAM, LPARAM);
/* See Undocumented windows p. 420 */

/* Константы ControlPanelInfo */
#define CPI_GETBEEP         1
#define CPI_SETBEEP         2
#define CPI_GETMOUSE        3
#define CPI_SETMOUSE        4
#define CPI_GETBORDER       5
#define CPI_SETBORDER       6
#define CPI_GETKEYBOARDSPEED 10
#define CPI_SETKEYBOARDSPEED 11
#define CPI_LANGDRIVER      12
#define CPI_ICONSPACING     13

/* Прототип недокументированной функции */
VOID FAR PASCAL ControlPanelInfo(int nInfoType, WORD wData, LPSTR lpBuffer);


#endif
