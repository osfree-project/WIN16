#pragma code_seg( "DEBUG_TEXT" )

#ifndef MK_FP
#define MK_FP(seg,off) ((void far *)(((unsigned long)(seg) << 16) | (unsigned)(off)))
#endif

/* Глобальная переменная для выбора порта вывода */
int comport = 1; /* 0 = консоль, 1 = COM1, 2 = COM2 */

/* Вывод одного символа через консоль или COM-порт */
void putchar(char c);
#pragma aux putchar = \
        "push ds"                 \
        "mov  ax, seg comport"    \
        "mov  ds, ax"             \
        "cmp  word ptr [comport], 0" \
        "je   putchar_console"     \
        "cmp  word ptr [comport], 1" \
        "je   putchar_com1"        \
        "cmp  word ptr [comport], 2" \
        "je   putchar_com2"        \
        "putchar_console:"        \
        "pop  ds"                 \
        "mov  ah, 2"               \
        "int  21h"                 \
        "jmp  putchar_end"         \
        "putchar_com1:"           \
        "pop  ds"                 \
        "push ax"                 \
        "push dx"                 \
        "push bx"                 \
        "mov  bl, dl"              \
        "com1_wait:"              \
        "mov  dx, 0x3FD"           \
        "in   al, dx"              \
        "test al, 0x20"           \
        "jz   com1_wait"            \
        "mov  dx, 0x3F8"           \
        "mov  al, bl"              \
        "out  dx, al"              \
        "pop  bx"                  \
        "pop  dx"                  \
        "pop  ax"                  \
        "jmp  putchar_end"         \
        "putchar_com2:"           \
        "pop  ds"                 \
        "push ax"                 \
        "push dx"                 \
        "push bx"                 \
        "mov  bl, dl"              \
        "com2_wait:"              \
        "mov  dx, 0x2FD"           \
        "in   al, dx"              \
        "test al, 0x20"           \
        "jz   com2_wait"            \
        "mov  dx, 0x2F8"           \
        "mov  al, bl"              \
        "out  dx, al"              \
        "pop  bx"                  \
        "pop  dx"                  \
        "pop  ax"                  \
        "putchar_end:"            \
        parm [dl]                 \
        modify [ax bx dx]

/* Преобразование 16-битного числа в строку по заданному основанию */
static char far * convert_to_ascii(char far *buf, int c, unsigned int num)
{
    unsigned int mult = (c == 'x' || c == 'X') ? 16 : 10;
    char far *ptr = buf;
    char far *p1;
    char far *p2;
    char tmp;

    if ((num & 0x8000) && c == 'd') {
        num = (~num) + 1;
        *(ptr++) = '-';
        buf++;
    }

    if (num == 0) {
        *(ptr++) = '0';
    } else {
        do {
            unsigned int dig = num % mult;
            *(ptr++) = (dig > 9) ? (char)(dig + 'a' - 10) : (char)('0' + dig);
        } while ((num /= mult) != 0);
    }
    *ptr = 0;

    p1 = buf;
    p2 = ptr - 1;
    while (p1 < p2) {
        tmp = *p1;
        *p1 = *p2;
        *p2 = tmp;
        p1++;
        p2--;
    }
    return buf;
}

/* Преобразование 32-битного числа в строку (для long) */
static char far * convert_to_ascii_long(char far *buf, int c, unsigned long num)
{
    char far *ptr = buf;
    int i, started, digit;
    unsigned long powers10[10];
    char far *p1;
    char far *p2;
    char tmp;

    if (c == 'x' || c == 'X') {
        started = 0;
        for (i = 7; i >= 0; i--) {
            int nibble = (num >> (i * 4)) & 0xF;
            if (nibble != 0 || started || i == 0) {
                *(ptr++) = (char)((nibble < 10) ? '0' + nibble : (c == 'X' ? 'A' + nibble - 10 : 'a' + nibble - 10));
                started = 1;
            }
        }
        if (!started) *(ptr++) = '0';
        *ptr = 0;
        return buf;
    }
    else if (c == 'u' || c == 'd') {
        if ((long)num < 0 && c == 'd') {
            num = (unsigned long)(-(long)num);
            *(ptr++) = '-';
            buf++;
        }

        powers10[0] = 1000000000UL;
        powers10[1] = 100000000UL;
        powers10[2] = 10000000UL;
        powers10[3] = 1000000UL;
        powers10[4] = 100000UL;
        powers10[5] = 10000UL;
        powers10[6] = 1000UL;
        powers10[7] = 100UL;
        powers10[8] = 10UL;
        powers10[9] = 1UL;

        started = 0;
        for (i = 0; i < 10; i++) {
            digit = 0;
            while (num >= powers10[i]) {
                num -= powers10[i];
                digit++;
            }
            if (digit != 0 || started || i == 9) {
                *(ptr++) = (char)('0' + digit);
                started = 1;
            }
        }
        *ptr = 0;
        p1 = buf;
        p2 = ptr - 1;
        while (p1 < p2) {
            tmp = *p1;
            *p1 = *p2;
            *p2 = tmp;
            p1++;
            p2--;
        }
        return buf;
    }
    return buf;
}

/* Вывод far-строки */
static void putstrfar(const char far *str)
{
    while (*str) putchar(*str++);
}

/* Основная функция форматированного вывода */
void _cdecl far printf(char far *format, ...)
{
    char far *arg_ptr = (char far *)&format;
    char c;
    char str[32];
    int width, precision, zero_pad, is_long, len, padding, i;
    unsigned int num;
    unsigned long num_long;
    char far *far_ptr;
    int offset, segment;
    char spec;
    int min_digits, total_len;
    unsigned short near_ptr_val;  /* Для %s */

    arg_ptr += 4;

    while ((c = *format++) != 0) {
        if (c != '%') {
            putchar(c);
            continue;
        }

        width = 0;
        precision = -1;
        zero_pad = 0;
        is_long = 0;

        c = *format;
        if (c == '0') {
            zero_pad = 1;
            format++;
            c = *format;
        }

        while (c >= '0' && c <= '9') {
            width = width * 10 + (c - '0');
            format++;
            c = *format;
        }

        if (c == '.') {
            format++;
            c = *format;
            precision = 0;
            while (c >= '0' && c <= '9') {
                precision = precision * 10 + (c - '0');
                format++;
                c = *format;
            }
        }

        if (c == 'l') {
            is_long = 1;
            format++;
            c = *format;
        }

        spec = c;
        format++;

        switch (spec) {
            case 'd':
            case 'u':
            case 'x':
            case 'X':
                if (is_long) {
                    num_long = *(unsigned long far *)arg_ptr;
                    arg_ptr += 4;
                    convert_to_ascii_long((char far *)str, spec, num_long);
                } else {
                    num = *(unsigned int far *)arg_ptr;
                    arg_ptr += 2;
                    convert_to_ascii((char far *)str, spec, num);
                }

                len = 0;
                while (((char far *)str)[len]) len++;

                /* Точность только для десятичных чисел */
                if (precision >= 0 && (spec == 'd' || spec == 'u')) {
                    if (precision > len) {
                        padding = precision - len;
                        for (i = len; i >= 0; i--)
                            ((char far *)str)[i + padding] = ((char far *)str)[i];
                        for (i = 0; i < padding; i++)
                            ((char far *)str)[i] = '0';
                        len += padding;
                    } else if (precision == 0 && ((char far *)str)[0] == '0' && len == 1) {
                        ((char far *)str)[0] = 0;
                        len = 0;
                    }
                    zero_pad = 0;
                }

                /* Ширина с флагом '0' */
                if (width > len) {
                    padding = width - len;
                    if (zero_pad) {
                        if (((char far *)str)[0] == '-') {
                            putchar('-');
                            for (i = 1; i < padding + 1; i++)
                                putchar('0');
                            putstrfar((const char far *)str + 1);
                        } else {
                            for (i = 0; i < padding; i++)
                                putchar('0');
                            putstrfar((const char far *)str);
                        }
                    } else {
                        for (i = 0; i < padding; i++)
                            putchar(' ');
                        putstrfar((const char far *)str);
                    }
                } else {
                    putstrfar((const char far *)str);
                }
                break;

            case 'c':
                c = *(char far *)arg_ptr;
                arg_ptr += 2;
                if (width > 1) {
                    for (i = 1; i < width; i++)
                        putchar(' ');
                }
                putchar(c);
                break;

            case 's':
                /* Для совместимости с любой моделью памяти: читаем 2 байта (near-указатель) */
                near_ptr_val = *(unsigned short far *)arg_ptr;
                arg_ptr += 2;
                {
                    /* Работаем как с обычным near-указателем, полагаясь, что DS правильный */
                    char *s_near = (char *)near_ptr_val;
                    len = 0;
                    if (precision >= 0) {
                        while (len < precision && s_near[len])
                            len++;
                    } else {
                        while (s_near[len])
                            len++;
                    }
                    if (width > len) {
                        for (i = len; i < width; i++)
                            putchar(' ');
                    }
                    for (i = 0; i < len; i++)
                        putchar(s_near[i]);
                }
                break;

            case 'S':
                offset = *(unsigned int far *)arg_ptr;
                arg_ptr += 2;
                segment = *(unsigned int far *)arg_ptr;
                arg_ptr += 2;
                far_ptr = MK_FP(segment, offset);
                len = 0;
                if (precision >= 0) {
                    while (len < precision && far_ptr[len])
                        len++;
                } else {
                    while (far_ptr[len])
                        len++;
                }
                if (width > len) {
                    for (i = len; i < width; i++)
                        putchar(' ');
                }
                for (i = 0; i < len; i++)
                    putchar(far_ptr[i]);
                break;

            case 'p':
                num = *(unsigned int far *)arg_ptr;
                arg_ptr += 2;
                convert_to_ascii((char far *)str, 'x', num);
                len = 0;
                while (((char far *)str)[len]) len++;
                min_digits = (precision > 0) ? precision : 4;
                total_len = 2 + (len < min_digits ? min_digits : len);
                if (width > total_len) {
                    for (i = total_len; i < width; i++)
                        putchar(' ');
                }
                putstrfar((const char far *)"0x");
                for (i = len; i < min_digits; i++)
                    putchar('0');
                putstrfar((const char far *)str);
                break;

            case 'F':
                if (*format == 'p' || *format == 'P') {
                    format++;
                    offset = *(unsigned int far *)arg_ptr;
                    arg_ptr += 2;
                    segment = *(unsigned int far *)arg_ptr;
                    arg_ptr += 2;

                    convert_to_ascii((char far *)str, 'x', segment);
                    len = 0;
                    while (((char far *)str)[len]) len++;
                    putstrfar((const char far *)"0x");
                    for (i = len; i < 4; i++)
                        putchar('0');
                    putstrfar((const char far *)str);
                    putchar(':');

                    convert_to_ascii((char far *)str, 'x', offset);
                    len = 0;
                    while (((char far *)str)[len]) len++;
                    putstrfar((const char far *)"0x");
                    for (i = len; i < 4; i++)
                        putchar('0');
                    putstrfar((const char far *)str);
                }
                else if (*format == 's' || *format == 'S') {
                    format++;
                    offset = *(unsigned int far *)arg_ptr;
                    arg_ptr += 2;
                    segment = *(unsigned int far *)arg_ptr;
                    arg_ptr += 2;
                    far_ptr = MK_FP(segment, offset);
                    len = 0;
                    if (precision >= 0) {
                        while (len < precision && far_ptr[len])
                            len++;
                    } else {
                        while (far_ptr[len])
                            len++;
                    }
                    if (width > len) {
                        for (i = len; i < width; i++)
                            putchar(' ');
                    }
                    for (i = 0; i < len; i++)
                        putchar(far_ptr[i]);
                }
                else {
                    putchar('%');
                    putchar('F');
                }
                break;

            case 'P':
                offset = *(unsigned int far *)arg_ptr;
                arg_ptr += 2;
                segment = *(unsigned int far *)arg_ptr;
                arg_ptr += 2;

                convert_to_ascii((char far *)str, 'x', segment);
                len = 0;
                while (((char far *)str)[len]) len++;
                putstrfar((const char far *)"0x");
                for (i = len; i < 4; i++)
                    putchar('0');
                putstrfar((const char far *)str);
                putchar(':');

                convert_to_ascii((char far *)str, 'x', offset);
                len = 0;
                while (((char far *)str)[len]) len++;
                putstrfar((const char far *)"0x");
                for (i = len; i < 4; i++)
                    putchar('0');
                putstrfar((const char far *)str);
                break;

            case '%':
                putchar('%');
                break;

            default:
                putchar('%');
                putchar(spec);
                break;
        }
    }
}

#pragma code_seg()
