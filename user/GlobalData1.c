#include "user.h"

WORD USER_HeapSel = 0;  /* USER heap selector */
char DebugBuffer[100];  /* Buffer for DEBUG messages */

char szSysError[0x14];
char szDivZero[0x14];
char szUntitled[0x14];
char szError[0x14];
char szOk[0x14];
char szCancel[0x14];
char szAbort[0x14];
char szRetry[0x14];
char szIgnore[0x14];
char szYes[0x14];
char szNo[0x14];
//char szClose[0x14]; not found in user.exe resources
char szAm[0x14];
char szPm[0x14];

int CBEntries;
int DefQueueSize;

char DISPLAY[]="DISPLAY";

HDC tempHDC;
