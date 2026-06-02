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

/* Color */
#define IDC_COL_ELEMENTS    1100
#define IDC_COL_SAMPLE      1101
#define IDC_COL_CHANGE      1102
#define IDC_COL_DEFCOLORS   1103
#define IDC_COL_SCHEMES     1104

/* Fonts */
#define IDC_FNT_LIST        1200
#define IDC_FNT_SAMPLE      1201
#define IDC_FNT_ADD         1202
#define IDC_FNT_REMOVE      1203

/* Ports */
#define IDC_PRT_LIST        1300
#define IDC_PRT_SETTINGS    1301
#define IDC_PRT_COM_BAUD    1302
#define IDC_PRT_COM_BITS    1303
#define IDC_PRT_COM_PARITY  1304
#define IDC_PRT_COM_STOP    1305
#define IDC_PRT_COM_FLOW    1306

/* Mouse Ц идентификаторы соответствуют ресурсу DLG_MOUSE */
#define IDC_MS_SPEED         532
#define IDC_MS_DOUBLECLICK   531
#define IDC_MS_SWAP          500
#define IDC_MS_L             503
#define IDC_MS_R             504
#define IDC_MS_TEST          529
#define IDC_MS_BUTTON_FRAME  505
#define IDC_MS_L_FRAME       506
#define IDC_MS_R_FRAME       507

/* Desktop */
#define IDC_DT_PATTERN      1500
#define IDC_DT_WALLPAPER    1501
#define IDC_DT_BROWSE_WP    1502
#define IDC_DT_TILE         1503

/* Keyboard */
#define IDC_KB_DELAY        1600
#define IDC_KB_RATE         1601
#define IDC_KB_TEST         1602

/* Printers */
#define IDC_PRN_LIST        1700
#define IDC_PRN_CONNECT     1701
#define IDC_PRN_SETUP       1702
#define IDC_PRN_REMOVE      1703

/* International */
#define IDC_INTL_COUNTRY     1800
#define IDC_INTL_LANGUAGE    1801
#define IDC_INTL_MEASURE     1802
#define IDC_INTL_LISTSEP     1803
#define IDC_INTL_DATEFORMAT  1804
#define IDC_INTL_CURRENCY    1805
#define IDC_INTL_DIGITS      1806
#define IDC_INTL_LEADZERO    1807
#define IDC_INTL_TIMEFORMAT  1808
#define IDC_INTL_SAMPLE_DATE 1809
#define IDC_INTL_SAMPLE_CURR 1810
#define IDC_INTL_SAMPLE_NUM  1811

/* Date/Time */
#define IDC_DT_DATE_TEXT     1900
#define IDC_DT_TIME_TEXT     1901
#define IDC_DT_MONTH_PREV    1902
#define IDC_DT_MONTH_NEXT    1903
#define IDC_DT_MONTH_TITLE   1904
#define IDC_DT_CALENDAR      1905
#define IDC_DT_HOUR_UP       1906
#define IDC_DT_HOUR_DOWN     1907
#define IDC_DT_MIN_UP        1908
#define IDC_DT_MIN_DOWN      1909
#define IDC_DT_SEC_UP        1910
#define IDC_DT_SEC_DOWN      1911

/* Network */
#define IDC_NET_MSG          2000

/* Icons (не используютс€, но оставлены дл€ совместимости) */
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

BOOL CALLBACK MouseDlgProc(HWND, UINT, WPARAM, LPARAM);

#endif
