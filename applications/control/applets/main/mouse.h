/* Mouse – идентификаторы соответствуют ресурсу DLG_MOUSE */
#define IDC_MS_SPEED         532
#define IDC_MS_DOUBLECLICK   531
#define IDC_MS_SWAP          500
#define IDC_MS_L             503
#define IDC_MS_R             504
#define IDC_MS_TEST          529
#define IDC_MS_BUTTON_FRAME  505
#define IDC_MS_L_FRAME       506
#define IDC_MS_R_FRAME       507

#define IDI_MOUSE  110

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
