#include <windows.h>

#define GlobalPtrHandle(lp) \
  ((HGLOBAL)LOWORD(GlobalHandle(SELECTOROF(lp))))

#define     GlobalUnlockPtr(lp)      \
                GlobalUnlock(GlobalPtrHandle(lp))

#define GlobalFreePtr(lp) \
  (GlobalUnlockPtr(lp),(BOOL)GlobalFree(GlobalPtrHandle(lp)))

#define GlobalAllocPtr(flags, cb) \
  (GlobalLock(GlobalAlloc((flags), (cb))))

typedef struct tagOBJHEAD
{
    HANDLE	hObj;			/* object handle */
//    WORD	wObjSignature;		/* object signature */
    WORD	wRefCount;		/* reference count */
//    struct tagOBJHEAD far *lpObjNext;	/* pointer to next */
} OBJHEAD;
typedef OBJHEAD far *LPOBJHEAD;

typedef struct {
	LPBYTE	rcsdata;	/* INTEL data */
	WORD	wType;		/* index into conversion routines table	*/
	HGLOBAL	hGlobal;	/* memory handle of resource	   */
	DWORD	rcsoffset;	/* location in resource file       */
	DWORD	rcslength;	/* length of data in resource file */
	WORD	rcsflags;	/* pre-loaded, discardable...      */
	HRSRC	hRsrc;		/* handle to 'found' resource      */
	HGDIOBJ	hObject;	/* GDI object associated with resource */
	LPSTR	rcsitemname;	/* name/ordinal of resource        */
} NAMEINFO;
typedef NAMEINFO far *LPNAMEINFO;

typedef struct tagMEMORYINFO
{
        OBJHEAD	ObjHead;	/* generic object header */
	struct  tagMEMORYINFO *lpNext;
	HMODULE hModule;	/* owning module */
	HTASK	hTask;		/* owning task */
	WORD	wFlags;		/* flags passed/set on handle */
	WORD	wType;		/* MT_ kind */
	DWORD	dwSize;		/* global object size */
	LPSTR   lpCore;		/* this points to native data */
	HANDLE	hMemory;	/* this is the memory handle  */
	WORD	wRefCount;	/* reference count */
	LPSTR	lpData;		/* points to resource data in 86-format */
	DWORD	dwBinSize;	/* size of object in lpData	*/
	WORD	wIndex;		/* resource type */

} MEMORYINFO;
typedef MEMORYINFO far *LPMEMORYINFO;


HBITMAP WINAPI
LoadBitmap(HINSTANCE hInstance, LPCSTR lpszBitmap)
{
	HANDLE hResNameInfo;
	LPNAMEINFO  lpTemplate;
	LPMEMORYINFO    lpMemory = (LPMEMORYINFO)NULL;

//    	APISTR((LF_APICALL,"LoadBitmap(HINSTANCE=%x,LPCSTR=%p)\n",
//		hInstance,lpszBitmap));

	hResNameInfo = FindResource(hInstance, lpszBitmap, RT_BITMAP);

	if(hResNameInfo == 0) {
//    		APISTR((LF_APIFAIL,"LoadBitmap: returns HBITMAP %x\n",0));
		return 0;
	}

	lpTemplate = (LPNAMEINFO) MAKELP(hResNameInfo,0);

	if(lpTemplate == 0) {
//    		APISTR((LF_APIFAIL,"LoadBitmap: returns HBITMAP %x\n",0));
		return 0;
	}
	if (lpTemplate->hGlobal == 0) {
//@tofo fix it
	    lpMemory = (LPMEMORYINFO)lpTemplate->rcsdata/*,MT_RESOURCE*/);
	    lpTemplate->hGlobal = lpMemory->hMemory;
	}
	if (lpTemplate->rcsdata == 0)
	    LoadResourceEx(hInstance,lpTemplate,lpMemory);

	if(!lpTemplate->hObject)
	    lpTemplate->hObject = (HGDIOBJ)CreateDIBitmapEx(
			(LPBITMAPIMAGE)lpTemplate->rcsdata,lpTemplate);
	else
	    LOCKGDI(lpTemplate->hObject);

//    	APISTR((LF_APIRET,"LoadBitmap: returns HBITMAP %x\n",lpTemplate->hObject));

	return (HBITMAP)lpTemplate->hObject;
}
