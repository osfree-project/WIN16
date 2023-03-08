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

/*
struct CLASS{
                HANDLE hNext;
                WORD wSign;
                ATOM atomClass;
                HANDLE hDCE;
                WORD wNumWin;
                WNDCLASS wndClass;
                WORD wData[1];
};
����:
hNext   ����� ��������� ��������� CLASS � �������� ������ USER.
wSign   ��������� 'NK'.
atomCls ���� � �������� ������ USER, ���������� ��� ������.
hDCE    ���� ��� ���� ������ ����������� CS_CLASSDC, �� ��� ���������� ��������������� ��������� DCE, ����� NULL.
wNumWin ������� ���������� ���� ����� ������.
wndClass        ����� ��������������� ��������� WNDCLASS � ����������� ������ lpszMenuName � lpszClassName.
wData   �������������� ������ ���������� ���� ???.

typedef struct tagWNDCLASS {
    UINT        style;
    WNDPROC     lpfnWndProc;
    int         cbClsExtra;
    int         cbWndExtra;
    HINSTANCE   hInstance;
    HICON       hIcon;
    HCURSOR     hCursor;
    HBRUSH      hbrBackground;
    LPCSTR      lpszMenuName;
    LPCSTR      lpszClassName;
} WNDCLASS;
typedef WNDCLASS        *PWNDCLASS;
typedef WNDCLASS NEAR   *NPWNDCLASS;
typedef WNDCLASS FAR    *LPWNDCLASS;

*/


typedef	struct	tagCLASSINFO {
    struct tagCLASSINFO	far * lpClassNext;	/* ptr to next class */
    struct tagCLASSINFO	far * lpClassPrev;	/* ptr to prev class */
    WORD wSign;	/* 'NK' */
    ATOM	atmClassName;		/* class name atom */
    HDC		hDC;			/* class DC handle */
    int		nUseCount;		/* Usage count  - private */
    WNDCLASS	wndClass;

    UINT	wClassType;		/* class type */

//    UINT	style;			/* class style */
//    WNDPROC	lpfnWndProc;		/* class procedure */
// This part is non-standard and seems to be translation  Native <> Binary layer
//    WNDPROC	lpfnBinToNat;		/* BINTONAT conversion procedure */
//    WNDPROC	lpfnNatToBin;		/* NATTOBIN conversion procedure */
//    int		cbClsExtra;		/* class extra bytes */
//    int		cbWndExtra;		/* window extra bytes */
//    HANDLE	hModule;		/* class module instance handle */
//    HICON	hIcon;			/* class icon resource handle */
//    HCURSOR	hCursor;		/* class cursor resource handle */
//    HBRUSH	hbrBackground;		/* class backgr. brush handle */
//    LPSTR	lpMenuName;		/* menu name string */

    LPSTR	lpClsExtra;		/* ptr to class extra bytes */
// No such in Win 3.x
//    HICON	hIconSm;		/* (WIN32) small class icon */
} CLASSINFO;
typedef CLASSINFO	*PCLASSINFO;
typedef CLASSINFO NEAR	*NPCLASSINFO;
typedef CLASSINFO FAR	*LPCLASSINFO;


