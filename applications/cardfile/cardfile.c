/*!

   (c) osFree Project 2002-2026, <https://www.osFree.org>
 
   SPDX-License-Identifier: BSD-3-Clause

*/

/* cardfile.c */
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include "cardfile.h"

CardNode *firstcard = NULL;
CardNode *lastcard  = NULL;
CardNode *topcard   = NULL;

/*--------------------------------------------------------------------*/
CardNode* CreateCardNode(const char *index, const char *text)
{
    CardNode *node = (CardNode*) malloc(sizeof(CardNode));
    int len;
    if (!node) return NULL;
    memset(node, 0, sizeof(CardNode));

    /* Безопасное копирование индекса (C89) */
    len = strlen(index);
    if (len > MAX_INDEX_LEN) len = MAX_INDEX_LEN;
    memcpy(node->data.index, index, len);
    node->data.index[len] = '\0';

    if (text) {
        len = strlen(text);
        node->data.text = (char*) malloc(len + 1);
        if (node->data.text) {
            memcpy(node->data.text, text, len);
            node->data.text[len] = '\0';
            node->data.textlen = len;
        } else {
            node->data.textlen = 0;
        }
    } else {
        node->data.text = NULL;
        node->data.textlen = 0;
    }
    return node;
}

/*--------------------------------------------------------------------*/
void FreeCardNode(CardNode *node)
{
    if (node) {
        if (node->data.text) free(node->data.text);
        free(node);
    }
}

/*--------------------------------------------------------------------*/
int CompareCards(const Card *a, const Card *b)
{
    char a_up[MAX_INDEX_LEN+1], b_up[MAX_INDEX_LEN+1];
    int i;
    for (i = 0; i <= MAX_INDEX_LEN; i++) {
        a_up[i] = (char)toupper((unsigned char)a->index[i]);
        b_up[i] = (char)toupper((unsigned char)b->index[i]);
    }
    {
        int cmp = lstrcmp(a_up, b_up);
        if (cmp != 0) return cmp;
    }
    return lstrcmp(a->index, b->index);
}

/*--------------------------------------------------------------------*/
int CompareCardWithStr(const Card *a, const char *str)
{
    Card tmp;
    memset(&tmp, 0, sizeof(tmp));
    {
        int len = strlen(str);
        if (len > MAX_INDEX_LEN) len = MAX_INDEX_LEN;
        memcpy(tmp.index, str, len);
        tmp.index[len] = '\0';
    }
    return CompareCards(a, &tmp);
}

/*--------------------------------------------------------------------*/
BOOL CardIndexEqual(const Card *a, const char *str)
{
    return (CompareCardWithStr(a, str) == 0);
}

/*--------------------------------------------------------------------*/
void InsertCardSorted(CardNode **nodePtr)
{
    CardNode *node = *nodePtr;
    if (!firstcard) {
        firstcard = lastcard = topcard = node;
        node->prev = node->next = node;
        return;
    }

    {
        CardNode *cur = topcard;
        if (CompareCards(&node->data, &cur->data) <= 0) {
            while (CompareCards(&node->data, &cur->prev->data) <= 0 &&
                   cur->prev != topcard)
                cur = cur->prev;
        } else {
            while (CompareCards(&node->data, &cur->next->data) > 0 &&
                   cur->next != topcard)
                cur = cur->next;
            cur = cur->next;
        }

        node->next = cur;
        node->prev = cur->prev;
        cur->prev->next = node;
        cur->prev = node;
        if (cur == firstcard && CompareCards(&node->data, &firstcard->data) < 0)
            firstcard = node;
        if (cur == lastcard->next)
            lastcard = node;
        topcard = node;
    }
}

/*--------------------------------------------------------------------*/
void RemoveCardNode(CardNode *node)
{
    if (!node || !firstcard) return;
    if (node == firstcard && node == lastcard) {
        firstcard = lastcard = topcard = NULL;
    } else {
        if (node == firstcard) firstcard = node->next;
        if (node == lastcard)  lastcard  = node->prev;
        if (topcard == node)   topcard   = node->next;
        node->prev->next = node->next;
        node->next->prev = node->prev;
    }
    node->prev = node->next = NULL;
}

/*--------------------------------------------------------------------*/
CardNode* FindCardByIndex(const char *index)
{
    CardNode *cur = firstcard;
    if (!cur) return NULL;
    do {
        if (CardIndexEqual(&cur->data, index)) return cur;
        cur = cur->next;
    } while (cur != firstcard);
    return NULL;
}

/*--------------------------------------------------------------------*/
CardNode* FindCardByText(CardNode *start, const char *substr)
{
    CardNode *cur = start ? start : firstcard;
    if (!cur || !substr || !*substr) return NULL;
    do {
        if (cur->data.text && strstr(cur->data.text, substr))
            return cur;
        cur = cur->next;
    } while (cur != (start ? start : firstcard));
    return NULL;
}

/*--------------------------------------------------------------------*/
CardNode* GetRelativeCard(int offset)
{
    CardNode *cur = topcard;
    if (!cur) return NULL;
    if (offset > 0) while (offset--) cur = cur->next;
    else if (offset < 0) while (offset++) cur = cur->prev;
    return cur;
}

/*--------------------------------------------------------------------*/
void MoveTopCard(int offset)
{
    if (!topcard) return;
    if (offset > 0) while (offset--) topcard = topcard->next;
    else if (offset < 0) while (offset++) topcard = topcard->prev;
}

/*--------------------------------------------------------------------*/
void DeleteAllCards(void)
{
    CardNode *cur, *nxt;
    if (!firstcard) return;
    cur = firstcard;
    do {
        nxt = cur->next;
        FreeCardNode(cur);
        cur = nxt;
    } while (cur != firstcard);
    firstcard = lastcard = topcard = NULL;
}

/* ======== Чтение файла .crd (только MGC и RRG) ======== */
BOOL LoadCards(const char *filename)
{
    HFILE hf;
    OFSTRUCT of;
    BYTE sig[3];
    DWORD last_obj_id;
    WORD  numcards;
    WORD  i;

    hf = OpenFile(filename, &of, OF_READ);
    if (hf == HFILE_ERROR) return FALSE;

    if (_lread(hf, sig, 3) != 3) { _lclose(hf); return FALSE; }

    /* Принимаем только MGC и RRG; DKO не поддерживаем в Win16 */
    if (memcmp(sig, "MGC", 3) != 0 && memcmp(sig, "RRG", 3) != 0) {
        _lclose(hf);
        return FALSE;
    }

    if (_lread(hf, &last_obj_id, sizeof(DWORD)) != sizeof(DWORD) ||
        _lread(hf, &numcards, sizeof(WORD)) != sizeof(WORD))
    { _lclose(hf); return FALSE; }

    for (i = 0; i < numcards; i++)
    {
        LONG   data_pos;
        BYTE   flag;
        char   idx_buf[MAX_INDEX_LEN + 1];
        Card   card;
        LONG   entry_start = HEADER_SIZE + i * INDEX_ENTRY_SIZE;

        _llseek(hf, entry_start, SEEK_SET);
        _llseek(hf, INDEX_RESERVED, SEEK_CUR);
        if (_lread(hf, &data_pos, sizeof(LONG)) != sizeof(LONG)) break;
        if (_lread(hf, &flag, 1) != 1) break;
        memset(idx_buf, 0, sizeof(idx_buf));
        if (_lread(hf, idx_buf, INDEX_TEXT_SIZE) != INDEX_TEXT_SIZE) break;
        idx_buf[MAX_INDEX_LEN] = '\0';

        _llseek(hf, data_pos, SEEK_SET);
        memset(&card, 0, sizeof(card));
        if (_lread(hf, &card.flags, sizeof(WORD)) != sizeof(WORD)) break;
        if (_lread(hf, &card.object_id, sizeof(DWORD)) != sizeof(DWORD)) break;
        {
            DWORD ole;
            if (_lread(hf, &ole, sizeof(DWORD)) != sizeof(DWORD)) break;
        }
        if (_lread(hf, &card.char_width, sizeof(WORD)) != sizeof(WORD)) break;
        if (_lread(hf, &card.char_height, sizeof(WORD)) != sizeof(WORD)) break;
        if (_lread(hf, &card.rect, sizeof(RECT)) != sizeof(RECT)) break;
        if (_lread(hf, &card.obj_type, sizeof(WORD)) != sizeof(WORD)) break;

        {
            WORD text_len;
            if (_lread(hf, &text_len, sizeof(WORD)) != sizeof(WORD)) break;

            if (text_len > 0) {
                card.text = (char*) malloc(text_len + 1);
                if (card.text) {
                    if (_lread(hf, card.text, text_len) != text_len) {
                        free(card.text);
                        break;
                    }
                    card.text[text_len] = '\0';
                    card.textlen = text_len;
                }
            } else {
                card.text = NULL;
                card.textlen = 0;
            }
        }

        /* Копируем индекс */
        {
            int len = strlen(idx_buf);
            if (len > MAX_INDEX_LEN) len = MAX_INDEX_LEN;
            memcpy(card.index, idx_buf, len);
            card.index[len] = '\0';
        }

        {
            CardNode *node = (CardNode*) malloc(sizeof(CardNode));
            if (!node) {
                if (card.text) free(card.text);
                break;
            }
            memcpy(&node->data, &card, sizeof(Card));
            {
                CardNode *p = node;
                InsertCardSorted(&p);
            }
        }
    }

    _lclose(hf);
    if (firstcard) topcard = firstcard;
    return (i == numcards);
}

/* ======== Сохранение в формате MGC (ASCII) через Windows API ======== */
BOOL SaveCards(const char *filename)
{
    HFILE hf;
    OFSTRUCT of;
    WORD  numcards = 0;
    LONG  last_obj_id = 0;
    LONG *data_positions = NULL;
    LONG *data_sizes = NULL;
    WORD  i;
    CardNode *cur;

    if (firstcard) {
        cur = firstcard;
        do { numcards++; cur = cur->next; } while (cur != firstcard);
    }

    if (numcards > 0) {
        data_positions = (LONG*) malloc(numcards * sizeof(LONG));
        data_sizes     = (LONG*) malloc(numcards * sizeof(LONG));
        if (!data_positions || !data_sizes) {
            if (data_positions) free(data_positions);
            if (data_sizes)     free(data_sizes);
            return FALSE;
        }
        cur = firstcard;
        for (i = 0; i < numcards; i++) {
            data_sizes[i] = 24 + cur->data.textlen;
            cur = cur->next;
        }
        data_positions[0] = HEADER_SIZE + numcards * INDEX_ENTRY_SIZE;
        for (i = 1; i < numcards; i++) {
            data_positions[i] = data_positions[i-1] + data_sizes[i-1];
        }
    }

    hf = OpenFile(filename, &of, OF_CREATE | OF_WRITE);
    if (hf == HFILE_ERROR) {
        if (data_positions) free(data_positions);
        if (data_sizes)     free(data_sizes);
        return FALSE;
    }

    _lwrite(hf, "MGC", 3);
    _lwrite(hf, &last_obj_id, sizeof(DWORD));
    _lwrite(hf, &numcards, sizeof(WORD));

    if (firstcard) {
        cur = firstcard;
        for (i = 0; i < numcards; i++) {
            BYTE reserved[6] = {0};
            BYTE flag = 0;
            char idx_buf[MAX_INDEX_LEN + 1];
            BYTE null_byte = 0;

            _lwrite(hf, reserved, INDEX_RESERVED);
            _lwrite(hf, &data_positions[i], sizeof(LONG));
            _lwrite(hf, &flag, 1);

            memset(idx_buf, 0, sizeof(idx_buf));
            {
                int len = strlen(cur->data.index);
                if (len > MAX_INDEX_LEN) len = MAX_INDEX_LEN;
                memcpy(idx_buf, cur->data.index, len);
                for (; len < INDEX_TEXT_SIZE; len++) idx_buf[len] = ' ';
            }
            _lwrite(hf, idx_buf, INDEX_TEXT_SIZE);
            _lwrite(hf, &null_byte, 1);
            cur = cur->next;
        }
    }

    if (firstcard) {
        cur = firstcard;
        for (i = 0; i < numcards; i++) {
            DWORD ole_zero = 0;
            _lwrite(hf, &cur->data.flags, sizeof(WORD));
            _lwrite(hf, &cur->data.object_id, sizeof(DWORD));
            _lwrite(hf, &ole_zero, sizeof(DWORD));
            _lwrite(hf, &cur->data.char_width, sizeof(WORD));
            _lwrite(hf, &cur->data.char_height, sizeof(WORD));
            _lwrite(hf, &cur->data.rect, sizeof(RECT));
            _lwrite(hf, &cur->data.obj_type, sizeof(WORD));
            _lwrite(hf, &cur->data.textlen, sizeof(WORD));
            if (cur->data.text && cur->data.textlen > 0) {
                _lwrite(hf, cur->data.text, cur->data.textlen);
            }
            cur = cur->next;
        }
    }

    if (data_positions) free(data_positions);
    if (data_sizes)     free(data_sizes);
    _lclose(hf);
    return TRUE;
}
