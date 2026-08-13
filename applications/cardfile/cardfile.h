/* cardfile.h */
#ifndef CARDFILE_H
#define CARDFILE_H

#include <windows.h>

/* Идентификаторы меню (соответствуют оригиналу) */
#define IDM_NEW        10
#define IDM_OPEN       11
#define IDM_SAVE       12
#define IDM_SAVEAS     13
#define IDM_MERGE      14
#define IDM_COPY       20
#define IDM_CUT        21
#define IDM_PASTE      22
#define IDM_INDEX      23
#define IDM_ADD        30
#define IDM_DELETE     31
#define IDM_DUP        32
#define IDM_FIND       40
#define IDM_EXIT        5
#define IDM_ABOUT       6

/* Идентификаторы кнопок и полей */
#define IDC_LISTBOX    1001
#define IDC_EDIT       1002
#define BTN_FIRST      1100
#define BTN_PREV       1101
#define BTN_NEXT       1102
#define BTN_LAST       1103
#define IDD_SEARCH     200
#define IDD_ADD        201
#define IDD_INDEX      202
#define IDD_ABOUT      203
#define SRCH_C         300
#define SRCH_V         301
#define SRCH_X         302

/* Максимальная длина индекса (без завершающего нуля) – 40 символов */
#define MAX_INDEX_LEN  40

/* Размеры полей согласно спецификации KB99340 */
#define INDEX_ENTRY_SIZE 52          /* полный размер одной индексной записи */
#define INDEX_RESERVED   6           /* зарезервированные байты (должны быть 0) */
#define INDEX_FLAG_OFFSET 10        /* флаговый байт */
#define INDEX_TEXT_OFFSET 11        /* начало текста индекса */
#define INDEX_TEXT_SIZE   40        /* размер индексной строки */
#define INDEX_NULL_OFFSET 51        /* завершающий нулевой байт */

/* Заголовок файла */
#define HEADER_SIZE 9               /* 3 сигнатура + 4 last object ID + 2 кол-во карт */

/* Константы, отсутствующие в некоторых версиях Win16 SDK */
#ifndef LB_FINDSTRINGEXACT
#define LB_FINDSTRINGEXACT  0x01A2
#endif

#ifndef MB_ICONERROR
#define MB_ICONERROR        MB_ICONHAND
#endif

/* Структура карточки */
typedef struct {
    char   index[MAX_INDEX_LEN + 1]; /* индексная строка */
    char  *text;                     /* текст карточки */
    WORD   textlen;                  /* длина текста без нуля */

    /* Дополнительные поля из спецификации */
    WORD   flags;                    /* флаги (2 байта) */
    DWORD  object_id;               /* уникальный ID объекта */
    WORD   char_width;              /* ширина символа */
    WORD   char_height;             /* высота символа */
    RECT   rect;                    /* координаты объекта */
    WORD   obj_type;                /* тип объекта */
} Card;

/* Узел двунаправленного кольцевого списка */
typedef struct cardnode {
    struct cardnode *prev;
    struct cardnode *next;
    Card             data;
} CardNode;

/* Глобальные указатели списка */
extern CardNode *firstcard;
extern CardNode *lastcard;
extern CardNode *topcard;

/* ---------- Операции над списком ---------- */
CardNode* CreateCardNode(const char *index, const char *text);
void      FreeCardNode(CardNode *node);
void      InsertCardSorted(CardNode **node);
void      RemoveCardNode(CardNode *node);
CardNode* FindCardByIndex(const char *index);
CardNode* FindCardByText(CardNode *start, const char *substr);
CardNode* GetRelativeCard(int offset);
void      MoveTopCard(int offset);
void      DeleteAllCards(void);

/* ---------- Файловые операции ---------- */
BOOL LoadCards(const char *filename);
BOOL SaveCards(const char *filename);

/* ---------- Сравнение карточек ---------- */
int  CompareCards(const Card *a, const Card *b);
int  CompareCardWithStr(const Card *a, const char *str);
BOOL CardIndexEqual(const Card *a, const char *str);

#endif
