/* PAGE12.C – Device drivers chain (from page_12.pas) */

#include <stdio.h>
#include <string.h>
#include <dos.h>
#include "msd.h"

/* --------------------------------------------------------------------------
   Вспомогательная static-функция: вывод 16 бит в двоичном виде
   (аналог bin16 из оригинального модуля)
   -------------------------------------------------------------------------- */
static void print_bin16(unsigned value)
{
    int i;
    for (i = 15; i >= 0; i--)
        cprintf("%d", (value >> i) & 1);
}

/* ========================================================================== */
void page12(void)
{
    unsigned char header[18];
    unsigned xword1, xword2;
    int i;

    Caption1("Device      Units    Header       Attributes"
             "             Strategy     Interrupt");

    /* Начальный адрес заголовка драйвера NUL */
    xword1 = devseg;
    xword2 = devofs + 0x0022;

    Window(1, 4, twidth, tlength - 2);

    while (xword2 < 0xFFFF)
    {
        pause2();
        if (endit) return;

        /* Чтение 18 байт заголовка драйвера */
        for (i = 0; i < 18; i++)
            header[i] = peekb(xword1, xword2 + i);

        /* Вывод имени устройства или количества устройств */
        if ((header[5] & 0x80) == 0x00)
        {
            /* Символьное устройство – показываем число устройств */
            cprintf("            %5u", header[10]);
        }
        else
        {
            /* Блочное устройство – выводим имя (8 символов) */
            for (i = 10; i <= 17; i++)
                putchar(showchar(header[i]));
            cprintf("         ");
        }

        cprintf("    ");
        SegOfs(xword1, xword2);               /* адрес заголовка */
        cprintf("    ");
        print_bin16(((unsigned)header[4] << 8) | header[5]);  /* атрибуты */
        cprintf("    ");
        SegOfs(xword1, ((unsigned)header[7] << 8) | header[6]); /* strategy */
        cprintf("    ");
        SegOfs(xword1, ((unsigned)header[9] << 8) | header[8]); /* interrupt */
        cprintf("\r\n");

        /* Переход к следующему драйверу */
        xword1 = ((unsigned)header[3] << 8) | header[2];  /* сегмент */
        xword2 = ((unsigned)header[1] << 8) | header[0];  /* смещение */
    }
}
