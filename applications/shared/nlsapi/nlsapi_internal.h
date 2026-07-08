/*
 * nlsapi_internal.h - Внутренние функции и данные NLS API (не экспортируются)
 */
#ifndef NLSAPI_INTERNAL_H
#define NLSAPI_INTERNAL_H

#include <string.h>
#include "winnls.h"

#ifdef STATIC
#define DECLSPEC __export
#else
#define DECLSPEC __loadds
#endif

/* Таблица сопоставления кодов стран и языков в LCID */
typedef struct {
    int   iCountry;      /* телефонный код страны */
    char  szLang[4];     /* код языка (usa, eng, frn и т.д.) */
    LCID  lcid;
} KNOWN_LCID;

extern const KNOWN_LCID knownLCIDs[];
extern const int KNOWN_LCID_COUNT;   /* <-- вместо макроса */

/* Преобразование кода страны и языка в LCID (или фиктивный) */
LCID LookupLCID(int countryCode, const char FAR *szLang);

/* Извлечение параметра локали из SETUP.INF */
BOOL GetLocaleInfoFromInf(int iCountryCode, const char FAR *szLang,
                          LCTYPE LCType, LPSTR lpLCData, int cchData,
                          BOOL returnNumber);

/* Безопасное преобразование FAR-строки в целое */
int AtoiFar(const char FAR *s);

/* Безопасное копирование строки с ограничением длины */
void StringCopyN(LPSTR dest, LPCSTR src, int n);

/* Сравнение строк без учёта регистра (far) */
int MyStrnicmp(const char FAR *s1, const char FAR *s2, int n);

BOOL IsDigitsOnly(LPCSTR str, int len);

/* Парсер INF */
HFILE OpenSetupInf(void);
BOOL ParseCountryLine(LPCSTR line, LPSTR name, int nameSize, int FAR * lpCode, LPSTR lang, int langSize, LPSTR params, int paramsSize);

#endif
