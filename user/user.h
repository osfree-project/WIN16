#include <windows.h>

typedef struct tagWNDCLASSEX {
	UINT	cbSize;
	UINT	style;
	WNDPROC	lpfnWndProc;
	int	cbClsExtra;
	int	cbWndExtra;
	HANDLE	hInstance;
	HICON	hIcon;
	HCURSOR	hCursor;
	HBRUSH	hbrBackground;
	LPCSTR	lpszMenuName;
	LPCSTR	lpszClassName;
	HICON	hIconSm;
} WNDCLASSEX;

typedef WNDCLASSEX	*PWNDCLASSEX;
typedef WNDCLASSEX NEAR	*NPWNDCLASSEX;
typedef WNDCLASSEX FAR	*LPWNDCLASSEX;

typedef struct tagINTWNDCLASS {
                HANDLE hcNext;
                WORD wSig;
                ATOM atomCls;
                HANDLE hDCE;
                WORD cClsWnds;
                WNDCLASS wndClass;
                WORD wData[1];
} INTWNDCLASS;

/* Private class style bit */
#define	CS_SYSTEMGLOBAL	0x8000

#define	SYSLOCAL	0
#define	SYSGLOBAL	1
#define	APPLOCAL	2
#define	APPGLOBAL	3

typedef	struct	tagCLASSINFO {
    struct tagCLASSINFO	far * lpClassNext;	/* ptr to next class */
    struct tagCLASSINFO	far *lpClassPrev;	/* ptr to prev class */
    UINT	wClassType;		/* class type */
    UINT	style;			/* class style */
    WNDPROC	lpfnWndProc;		/* class procedure */
    WNDPROC	lpfnBinToNat;		/* BINTONAT conversion procedure */
    WNDPROC	lpfnNatToBin;		/* NATTOBIN conversion procedure */
    int		cbClsExtra;		/* class extra bytes */
    int		cbWndExtra;		/* window extra bytes */
    HANDLE	hModule;		/* class module instance handle */
    HICON	hIcon;			/* class icon resource handle */
    HCURSOR	hCursor;		/* class cursor resource handle */
    HBRUSH	hbrBackground;		/* class backgr. brush handle */
    HDC		hDC;			/* class DC handle */
    ATOM	atmClassName;		/* class name atom */
    int		nUseCount;		/* Usage count  - private */
    LPSTR	lpMenuName;		/* menu name string */
    LPSTR	lpClsExtra;		/* ptr to class extra bytes */
    HICON	hIconSm;		/* (WIN32) small class icon */
} CLASSINFO;
typedef CLASSINFO	*PCLASSINFO;
typedef CLASSINFO NEAR	*NPCLASSINFO;
typedef CLASSINFO FAR	*LPCLASSINFO;


