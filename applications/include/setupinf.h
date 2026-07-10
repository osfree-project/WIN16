#ifndef SETUPINF_H
#define SETUPINF_H

#include "common_types.h"

typedef struct INF_FILE_TAG    FAR *HINF;
typedef struct INF_SECTION_TAG FAR *LPINF_SECTION;
typedef LPINF_SECTION HINF_SECTION;

HINF InfOpen(LPCSTR filename);
void InfClose(HINF hInf);

LPINF_SECTION InfFindSection(HINF hInf, LPCSTR sectionName);
int InfGetLineCount(LPINF_SECTION hSection);
LPCSTR InfGetLine(LPINF_SECTION hSection, int index);

void InfFreeSection(HINF hInf, LPCSTR sectionName);
void InfClearAllCache(HINF hInf);

#endif
