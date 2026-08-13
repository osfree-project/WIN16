/* PAGE11.C – Program name and environment (from page_11.pas) */

#include <stdio.h>
#include <string.h>
#include <dos.h>
#include "msd.h"

/* Прототипы локальных функций (если нужны) */
static void show_environment(unsigned envseg);

/* ========================================================================== */
void page11(void)
{
    unsigned temp, temp1, envseg, envlen, envused;
    int foundit, endfound;

    Caption2("Program name");
    cprintf("%s\r\n", _argv[0]);   /* аналог ParamStr(0) */

    /* Поиск родительского PSP (обход цепочки PSP) */
    temp = peekw(PrefixSeg, 0x16);
    foundit = 0;
    while (!foundit)
    {
        temp1 = peekw(temp, 0x16);
        if (temp1 == 0 || temp1 == temp)
            foundit = 1;
        else
            temp = temp1;
    }

    /* Сегмент окружения */
    envseg = peekw(temp, 0x2C);
    if (envseg == 0 || (OSMinor > 19 && OSMinor < 30))
        envseg = temp + peekw(temp - 1, 3) + 1;

    /* Размер окружения (в байтах) */
    envlen = peekw(envseg - 1, 3) * 16;

    /* Подсчёт используемого места */
    envused = 0;
    endfound = 0;
    while (!endfound)
    {
        if (peekw(envseg, envused) == 0)  /* два нулевых байта подряд */
            endfound = 1;
        else
            envused++;
    }
    envused += 2;   /* завершающий ноль */

    Caption2("Environment\r\n");
    Caption3("Segment");
    cprintf("%04X", envseg);
    Caption3("Size");
    cprintf("%4u", envlen);
    Caption3("Used");
    cprintf("%4u", envused);
    Caption3("Free");
    cprintf("%4u\r\n", envlen - envused);

    Caption2("Variables");
    Window(3, 7, twidth, tlength - 2);

    /* Вывод всех переменных окружения */
    {
        unsigned pos = 0;
        while (pos < envused - 1)   /* не включая финальный ноль */
        {
            pause2();
            if (endit) return;

            /* Формируем строку до нуля */
            char buf[256];
            int i = 0;
            while (peekb(envseg, pos) != 0 && i < 255)
            {
                buf[i++] = peekb(envseg, pos++);
            }
            buf[i] = '\0';
            cprintf("%s\r\n", buf);
            pos++;   /* пропускаем нулевой байт */
            if (peekb(envseg, pos) == 0) break;   /* двойной ноль – конец */
        }
    }
}
