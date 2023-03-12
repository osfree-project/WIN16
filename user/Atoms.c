#include <string.h>
#include <ctype.h>

#include <windows.h>

#define GlobalPtrHandle(lp) \
  ((HGLOBAL)LOWORD(GlobalHandle(SELECTOROF(lp))))

#define     GlobalUnlockPtr(lp)      \
                GlobalUnlock(GlobalPtrHandle(lp))

#define GlobalFreePtr(lp) \
  (GlobalUnlockPtr(lp),(BOOL)GlobalFree(GlobalPtrHandle(lp)))

#define GlobalAllocPtr(flags, cb) \
  (GlobalLock(GlobalAlloc((flags), (cb))))

#define ATOMBASE	 0xcc00

typedef unsigned long ATOMID;

typedef struct {
	ATOMID	q;		/* what a string 'hashes' to 	*/
	long   	idx;		/* index into data table	*/
	long	refcnt;		/* how many clients have it 	*/
	long	idsize;		/* space used by this slot	*/
} ATOMENTRY;
typedef ATOMENTRY far *LPATOMENTRY;

typedef struct {
	ATOMENTRY far	*AtomTable;	/* pointer to table data 	*/
	char	far	*AtomData;	/* pointer to name data 	*/
	unsigned long	 TableSize;	/* number items in this table   */
	unsigned long	 DataSize;	/* space used by string data    */
	LPVOID		lpDrvData;
} ATOMTABLE;
typedef ATOMTABLE far *LPATOMTABLE;

static LPATOMTABLE GlobalTable;

static ATOMID
AtomHashString(LPCSTR lp,int far *lplen)
{
	ATOMID 	q;
	char  far *p,ch;
	int	len;

	/* if we have an intatom... */
	if(HIWORD(lp) == 0) {
		if(lplen) *lplen = 0;
		return (ATOMID)lp;
	}

	/* convert the string to an internal representation */
	for(p=(LPSTR)lp,q=0,len=0;(ch=*p++);len++)
		q = (q<<1) + islower(ch)?toupper(ch):ch;

	/* 0 is reserved for empty slots */
	if(q == 0)
		q++;

	/* avoid strlen later */
	if(lplen) {
		*lplen = ++len;
	}
	return q;
}


/********************************************************/
/*	convert an atom index into a pointer into an 	*/
/* 	atom table.  This validates the pointer is in   */
/*	range, and that the data is accessible		*/
/********************************************************/

static ATOMENTRY far *
GetAtomPointer(ATOMTABLE far *at,int index)
{
	ATOMENTRY far *lp;
	
	/* if no table, then no pointers */
	if(at->AtomTable == 0)
		return 0;

	/* bad index */
	if((index < 0) || (index >= at->TableSize))
		return 0;

	/* we have a pointer */
	lp = &at->AtomTable[index];

	/* is the index past stored data, validity check		*/
	/* LATER: is the size of the entry within the available space 	*/
	if(lp->idx > at->DataSize)
		return 0;

	return lp;
}

/***********************************************************************
 *		GlobalDeleteAtom (USER.269)
 */
ATOM WINAPI
GlobalDeleteAtom(ATOM atom)
{
	ATOMENTRY far *lp;
	
	/* a free slot has q == 0 && refcnt == 0 */
	if((lp = GetAtomPointer(GlobalTable,atom - ATOMBASE))) {
		if(lp->idsize)
			lp->refcnt--;

		if(lp->refcnt == 0) {
			return lp->q = 0;
		}
	}
	return atom;
}

/***********************************************************************
 *		GlobalGetAtomName (USER.271)
 */
UINT WINAPI
GlobalGetAtomName(ATOM atom,LPSTR lpszbuf,int len)
{
	ATOMENTRY far *lp;
	char 	 far  *atomstr;
	int	   atomlen;
	
	/* return the atom name, or create the INTATOM */
	if((lp = GetAtomPointer(GlobalTable,atom - ATOMBASE))) {
		if(lp->idsize) {
			atomlen = lstrlen(atomstr = &GlobalTable->AtomData[lp->idx]);
			if (atomlen < len)
			    lstrcpy(lpszbuf,atomstr);
			else {
			    lstrcpyn(lpszbuf,atomstr,len-1);
			    lpszbuf[len-1] = '\0';
			}
			return (UINT)lstrlen(lpszbuf);
		} else {
			wsprintf(lpszbuf,"#%d",lp->q);
			return (UINT)lstrlen(lpszbuf);
		}
	}
	return 0;
}

/***********************************************************************
 *		GlobalFindAtom (USER.270)
 */
ATOM WINAPI
GlobalFindAtom(LPCSTR lpstr)
{
	ATOMID		q;
	LPATOMENTRY   	lp;
	int		index;
	int		atomlen;

	/* convert string to 'q', and get length */
	q = AtomHashString(lpstr,&atomlen);

	/* find the q value, note: this could be INTATOM */
	/* if q matches, then do case insensitive compare*/
	for(index = 0;(lp = GetAtomPointer(GlobalTable,index));index++) {
		if(lp->q == q) {	
			if(HIWORD(lpstr) == 0)
				return ATOMBASE + index;
			if(_fstricmp(&GlobalTable->AtomData[lp->idx],lpstr) == 0)
				return ATOMBASE + index;
		}
	}
	return 0;
}

/***********************************************************************
 *		GlobalAddAtom (USER.268)
 */
ATOM WINAPI
GlobalAddAtom(LPCSTR lpstr)
{
	ATOM atom;
	ATOMID		q;
	LPATOMENTRY   	lp,lpfree;
	int		index,freeindex;
	int		atomlen;
	int		newlen;
	
	/* if we already have it, bump refcnt */
	if((atom = GlobalFindAtom(lpstr))) {
		lp = GetAtomPointer(GlobalTable,atom - ATOMBASE);
		if(lp->idsize) lp->refcnt++;
		return atom;
	}

	/* add to a free slot */
	q = AtomHashString(lpstr,&atomlen);

	lpfree 	  = 0;
	freeindex = 0;

	for(index = 0;(lp = GetAtomPointer(GlobalTable,index));index++) {
		if(lp->q == 0 && lp->refcnt == 0) {	
			if(lp->idsize > atomlen) {
				if ((lpfree == 0) ||
					    (lpfree->idsize > lp->idsize)) {
					lpfree = lp;
					freeindex = index;
				}
			}
		}
	}
	/* intatoms do not take space in data, but do get new entries */
	/* an INTATOM will have length of 0 			      */
	if(lpfree && atomlen) {
		lpfree->q = q;
		lpfree->refcnt = 1;			
		lstrcpyn(&GlobalTable->AtomData[lpfree->idx],lpstr,atomlen);
		return freeindex + ATOMBASE;
	}

	/* no space was available, or we have an INTATOM		*/
	/* so expand or create the table 				*/
	if(GlobalTable->AtomTable == 0) {
		GlobalTable->AtomTable = (ATOMENTRY far *) GlobalAllocPtr(GPTR, sizeof(ATOMENTRY));
		GlobalTable->TableSize = 1;
		lp = GlobalTable->AtomTable;
		index = 0;
	} else {
		GlobalTable->TableSize++;
		GlobalTable->AtomTable = (ATOMENTRY far *) GlobalLock(GlobalReAlloc(GlobalPtrHandle(
			(char far *) GlobalTable->AtomTable),
			GlobalTable->TableSize * sizeof(ATOMENTRY), GMEM_MOVEABLE));
		lp = &GlobalTable->AtomTable[GlobalTable->TableSize - 1];
	}

	/* set in the entry */
	lp->refcnt = 1;
	lp->q      = q;
	lp->idsize = atomlen;
	lp->idx    = 0;

	/* add an entry if not intatom... */
	if(atomlen) {
		newlen = GlobalTable->DataSize + atomlen;

		if(GlobalTable->AtomData == 0) {
			GlobalTable->AtomData = (char far *) GlobalAllocPtr(GPTR, newlen);
			lp->idx = 0;
		} else {
			GlobalTable->AtomData = (char far *) GlobalLock(GlobalReAlloc(GlobalPtrHandle(GlobalTable->AtomData),newlen, GMEM_MOVEABLE));
			lp->idx = GlobalTable->DataSize;
		}

		lstrcpy(&GlobalTable->AtomData[lp->idx],lpstr);
		GlobalTable->DataSize = newlen;
	}	

	return index + ATOMBASE;
}
