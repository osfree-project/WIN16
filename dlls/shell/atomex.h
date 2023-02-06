typedef struct
{
    WORD        size;
    HANDLE	entries[1];
} ATOMTABLE;

ATOM WINAPI DeleteAtomEx(ATOMTABLE far * atomtable, ATOM atom );
ATOM WINAPI FindAtomEx(ATOMTABLE far * atomtable, LPCSTR str );
UINT WINAPI GetAtomNameEx(ATOMTABLE far * atomtable, ATOM atom, LPSTR buffer, int count );
ATOM WINAPI AddAtomEx(ATOMTABLE far * atomtable, LPCSTR str );

