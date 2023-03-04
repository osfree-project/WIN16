#include <windows.h>

typedef

struct tagINTWNDCLASS {
                HANDLE hcNext;
                WORD wSig;
                ATOM atomCls;
                HANDLE hDCE;
                WORD cClsWnds;
                WNDCLASS wndClass;
                WORD wData[1];
} INTWNDCLASS;

/* Additional messages to the LISTBOX system class */
#define	LB_GETINDENTS		(WM_USER+40)
#define	LB_SETINDENTS		(WM_USER+41)
#define	LB_GETITEMINDENTS	(WM_USER+42)
#define	LB_SETITEMINDENTS	(WM_USER+43)
#define	LB_GETITEMBITMAPS	(WM_USER+44)
#define	LB_SETITEMBITMAPS	(WM_USER+45)
#define	LB_ADDITEM		(WM_USER+46)

/* Private listbox style bit */
#define	LBS_COMBOLBOX	0x8000
#define	LBS_PRELOADED	0x4000
