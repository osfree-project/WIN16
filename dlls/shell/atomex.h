typedef struct
{
    WORD        size;
    HANDLE	entries[1];
} ATOMTABLE;
typedef ATOMTABLE FAR *LPATOMTABLE;


ATOM WINAPI DeleteAtomEx(LPATOMTABLE atomtable, ATOM atom );
ATOM WINAPI FindAtomEx(LPATOMTABLE atomtable, LPCSTR str );
UINT WINAPI GetAtomNameEx(LPATOMTABLE atomtable, ATOM atom, LPSTR buffer, int count );
ATOM WINAPI AddAtomEx(LPATOMTABLE atomtable, LPCSTR str );

