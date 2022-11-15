#include <win16.h>

void WINAPI LongPtrAdd(DWORD dwLongPtr, DWORD dwAdd)
{
  WORD wSel = SELECTOROF(dwLongPtr);
  SetSelectorBase(wSel, GetSelectorBase(wSel)+dwAdd);
}

