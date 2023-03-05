
typedef struct tagMENUITEMSTRUCT
{
    WORD	wPosition;		/* position to insert */
    WORD	wIDNewItem;		/* command ID/menu handle */
    WORD	wAction;		/* action bits for change */
    HBITMAP	hCheckedBmp;		/* checkmark bitmap */
    HBITMAP	hUncheckedBmp;		/* unchecked bitmap */
    WORD	wLeftIndent;		/* left indent */
    WORD	wRightIndent;		/* right indent */
    WORD	wItemFlags;		/* item flags */
    LPSTR	lpItemData;		/* item contents */
} MENUITEMSTRUCT;

typedef MENUITEMSTRUCT far *LPMENUITEMSTRUCT;

typedef struct tagMENUCREATESTRUCT
{
    HFONT	hFont;
    DWORD	dwStyle;
    DWORD	dwIndents;
} MENUCREATESTRUCT;

typedef MENUCREATESTRUCT far *LPMENUCREATESTRUCT;

typedef struct tagTRACKPOPUPSTRUCT
{
    HMENU	hMenu;
    UINT	uiFlags;
    int		x;
    int		y;
    BOOL	bSystemMenu;
    HWND	hWndOwner;
    RECT    far *lprc;
    HWND	hPopups[5];	/* this should be a linked list */
    WORD	wPopupFlags[5];
    int		nPopups;	/* number of popups */
} TRACKPOPUPSTRUCT;

typedef TRACKPOPUPSTRUCT far *LPTRACKPOPUPSTRUCT;

#define	TP_STATUS	0

#define TP_MENUBAR	0x2000

#define     GetWindowInstance(hwnd) ((HINSTANCE)GetWindowWord(hwnd, GWW_HINSTANCE))

/* ChangeItem action bits */
#define	LCA_GET		0x0000
#define	LCA_SET		0x8000
#define	LCA_CONTENTS	0x0001
#define	LCA_CHECKBMP	0x0002
#define	LCA_UNCHECKBMP	0x0004
#define	LCA_LEFTINDENT	0x0008
#define	LCA_RIGHTINDENT	0x0010
#define	LCA_FLAGS	0x0020
#define	LCA_RECT	0x0040
#define	LCA_ITEMID	0x0080
#define	LCA_ITEMCOUNT	0x0100
#define	LCA_FONT	0x0200
#define	LCA_SELECTION	0x0400
#define	LCA_STATE	0x0800

#define	LCA_BITMAPS	(LCA_CHECKBMP | LCA_UNCHECKBMP)
#define	LCA_INDENTS	(LCA_LEFTINDENT | LCA_RIGHTINDENT)
#define	LCA_ALL		(LCA_INDENTS | LCA_BITMAPS | LCA_FLAGS | \
			LCA_CONTENTS | LCA_ITEMID)

#define LBA_CREATE	0
#define LBA_DESTROY	1
#define LBA_MODIFYITEM	2
#define LBA_INSERTITEM	3
#define LBA_APPENDITEM	4
#define LBA_DELETEITEM	5
#define LBA_REMOVEITEM	6
#define LBA_GETDATA	7
#define LBA_SETDATA	8
