#include "user.h"

HCURSOR WINAPI SetCursor(HCURSOR hCursor)
{
    CURSORICONINFO FAR * lpCursorInfo = NULL;
    LPCURSORSHAPE lpCursorShape = NULL;
    DWORD dwAndMaskSize = 0;
    DWORD dwXorMaskSize = 0;
    DWORD dwTotalSize = 0;
    BYTE FAR *lpBits = NULL;
    
    // Блокируем глобальный блок памяти курсора
    lpCursorInfo = (CURSORICONINFO FAR *)GlobalLock(hCursor);
    if (!lpCursorInfo) {
        return NULL;
    }
    
    // Вычисляем размеры масок
    dwAndMaskSize = (DWORD)lpCursorInfo->nHeight * lpCursorInfo->nWidthBytes;
    dwXorMaskSize = dwAndMaskSize; // Обе маски одинакового размера для монохромного
    
    // Выделяем память для структуры CURSORSHAPE
    dwTotalSize = sizeof(CURSORSHAPE) + dwAndMaskSize + dwXorMaskSize - 1;
    lpCursorShape = (LPCURSORSHAPE)GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT, dwTotalSize);
    if (!lpCursorShape) {
        GlobalUnlock(hCursor);
        return NULL;
    }
    
    // Заполняем заголовок структуры CURSORSHAPE
    lpCursorShape->csHotX = (WORD)lpCursorInfo->ptHotSpot.x;
    lpCursorShape->csHotY = (WORD)lpCursorInfo->ptHotSpot.y;
    lpCursorShape->csWidth = lpCursorInfo->nWidth;
    lpCursorShape->csHeight = lpCursorInfo->nHeight;
    lpCursorShape->csWidthBytes = lpCursorInfo->nWidthBytes;
    
    // csColor = 0 для монохромного
    lpCursorShape->csColor = 0;
    
    // Указатель на данные масок в исходном курсоре
    // Они идут сразу после структуры CURSORICONINFO
    lpBits = (BYTE FAR *)lpCursorInfo + sizeof(CURSORICONINFO);
    
    // Копируем AND маску (первая половина данных)
    _fmemcpy(lpCursorShape->csBits, lpBits, dwAndMaskSize);
    
    // Копируем XOR маску (вторая половина данных)
    _fmemcpy(lpCursorShape->csBits + dwAndMaskSize, 
             lpBits + dwAndMaskSize, 
             dwXorMaskSize);
    
    // Разблокируем исходный курсор
    GlobalUnlock(hCursor);
    
    // Вызываем DisplaySetCursor с нашей структурой
    DisplaySetCursor(lpCursorShape);
    
    // Освобождаем память структуры CURSORSHAPE
    // Предполагается, что DisplaySetCursor скопировала данные или использует их сразу
    GlobalFree((HGLOBAL)lpCursorShape);

	return hCursor;//@todo return previous cursor
}
