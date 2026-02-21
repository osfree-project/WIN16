
/*
This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, see
<https://www.gnu.org/licenses/>.
 *
 */

#pragma code_seg( "DEBUG_TEXT" );

#ifndef MK_FP
#define MK_FP(seg,off) ((void far *)(((unsigned long)(seg) << 16) | (unsigned)(off)))
#endif

/* This function prints one char */
//extern  void putchar(char);
//#pragma aux putchar               = 
//        "mov ah, 2"          
//        "int 21h"          
//        parm                   [dl];

/* Глобальная переменная для выбора порта вывода */
int comport = 1; /* 0 = консоль, 1 = COM1, 2 = COM2 */

/* This function prints one char */
extern  void putchar(char);
#pragma aux putchar               = \
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
        parm                   [dl] \
        modify                [ax bx dx];

#if 0
/* Глобальная переменная для выбора порта вывода */
int comport = 1; /* 0 = консоль, 1 = COM1, 2 = COM2 */

/* This function prints one char */
extern  void putchar(char);
#pragma aux putchar               = \
        "cmp word ptr [comport], 0" \
        "je  putchar_console"     \
        "cmp word ptr [comport], 1" \
        "je  putchar_com1"        \
        "cmp word ptr [comport], 2" \
        "je  putchar_com2"        \
        "putchar_console:"        \
        "mov ah, 2"               \
        "int 21h"                 \
        "jmp putchar_end"         \
        "putchar_com1:"           \
        "push ax"                 \
        "push dx"                 \
        "push bx"                 \
        "mov bl, dl"              \
        "com1_wait:"              \
        "mov dx, 0x3FD"           \
        "in  al, dx"              \
        "test al, 0x20"           \
        "jz com1_wait"            \
        "mov dx, 0x3F8"           \
        "mov al, bl"              \
        "out dx, al"              \
        "pop bx"                  \
        "pop dx"                  \
        "pop ax"                  \
        "jmp putchar_end"         \
        "putchar_com2:"           \
        "push ax"                 \
        "push dx"                 \
        "push bx"                 \
        "mov bl, dl"              \
        "com2_wait:"              \
        "mov dx, 0x2FD"           \
        "in  al, dx"              \
        "test al, 0x20"           \
        "jz com2_wait"            \
        "mov dx, 0x2F8"           \
        "mov al, bl"              \
        "out dx, al"              \
        "pop bx"                  \
        "pop dx"                  \
        "pop ax"                  \
        "putchar_end:"            \
        parm                   [dl];

#endif

/* Инициализация COM-порта через ассемблерную вставку */
void init_com(int port, int baud_rate)
{
    unsigned short divisor = 12; //115200 / baud_rate;
    
    if (port == 1) {
        /* Инициализация COM1 (0x3F8) */
        _asm {
            mov dx, 0x3F8
            add dx, 3
            mov al, 0x80
            out dx, al
            
            mov dx, 0x3F8
            mov al, byte ptr divisor
            out dx, al
            
            mov dx, 0x3F8
            inc dx
            mov al, byte ptr divisor+1
            out dx, al
            
            mov dx, 0x3F8
            add dx, 3
            mov al, 0x03
            out dx, al
            
            mov dx, 0x3F8
            add dx, 2
            mov al, 0xC7
            out dx, al
            
            mov dx, 0x3F8
            add dx, 4
            mov al, 0x03
            out dx, al
        }
    } else {
        /* Инициализация COM2 (0x2F8) */
        _asm {
            mov dx, 0x2F8
            add dx, 3
            mov al, 0x80
            out dx, al
            
            mov dx, 0x2F8
            mov al, byte ptr divisor
            out dx, al
            
            mov dx, 0x2F8
            inc dx
            mov al, byte ptr divisor+1
            out dx, al
            
            mov dx, 0x2F8
            add dx, 3
            mov al, 0x03
            out dx, al
            
            mov dx, 0x2F8
            add dx, 2
            mov al, 0xC7
            out dx, al
            
            mov dx, 0x2F8
            add dx, 4
            mov al, 0x03
            out dx, al
        }
    }
}

/* Изменяем convert_to_ascii - теперь она принимает num явно */
char * convert_to_ascii (char *buf, int c, unsigned int num)
{
  unsigned int mult = 10;
  char *ptr = buf;
  
  if (c == 'x' || c == 'X')
    mult = 16;

  if ((num & 0x8000) && c == 'd')
    {
      num = (~num) + 1;
      *(ptr++) = '-';
      buf++;
    }

  if (num == 0) {
    *(ptr++) = '0';
    *ptr = 0;
    return ptr;
  }

  do
    {
      int dig = num % mult;
      *(ptr++) = ((dig > 9) ? dig + 'a' - 10 : '0' + dig);
    }
  while (num /= mult);

  /* reorder to correct direction!! */
  {
    char *ptr1 = ptr - 1;
    char *ptr2 = buf;
    while (ptr1 > ptr2)
      {
        int tmp = *ptr1;
        *ptr1 = *ptr2;
        *ptr2 = tmp;
        ptr1--;
        ptr2++;
      }
  }

  *ptr = 0;
  return ptr;
}

/* Функция для преобразования 32-битного числа без деления */
char * convert_to_ascii_long (char *buf, int c, unsigned long num)
{
  unsigned long remainder;
  char *ptr = buf;
  int i, j;
  char tmp_char;
  
  if (c == 'x' || c == 'X')
  {
    /* Для шестнадцатеричного - простой сдвиг */
    for (i = 7; i >= 0; i--)
    {
      int nibble = (num >> (i * 4)) & 0xF;
      if (nibble != 0 || i == 0) /* Всегда выводим последнюю цифру */
      {
        if (ptr == buf && nibble == 0 && i > 0)
          continue; /* Пропускаем ведущие нули */
        *(ptr++) = (nibble < 10) ? ('0' + nibble) : 
                   ((c == 'X') ? ('A' + nibble - 10) : ('a' + nibble - 10));
      }
    }
    *ptr = 0;
    return ptr;
  }
  else if (c == 'u' || c == 'd')
  {
    /* Для десятичного - используем вычитание степеней 10 */
    const unsigned long powers10[] = {
      1000000000UL,
      100000000UL,
      10000000UL,
      1000000UL,
      100000UL,
      10000UL,
      1000UL,
      100,
      10,
      1
    };
    
    int started = 0;
    
    /* Специальная обработка для знаковых отрицательных */
    if (((long)num < 0) && c == 'd')
    {
      *(ptr++) = '-';
      num = (unsigned long)(-(long)num);
      buf++; /* Сдвигаем начало буфера, минуя минус */
    }
    
    for (i = 0; i < 10; i++)
    {
      int digit = 0;
      while (num >= powers10[i])
      {
        num -= powers10[i];
        digit++;
      }
      
      if (digit != 0 || started || i == 9) /* Последняя цифра всегда выводится */
      {
        *(ptr++) = '0' + digit;
        started = 1;
      }
    }
    
    *ptr = 0;
    return ptr;
  }
  
  return buf;
}

void putstr (const char *str)
{
  while (*str)
    putchar (*str++);
}

void putstrfar (const char far *str)
{
  while (*str)
    putchar (*str++);
}

void _cdecl far printf (char far *format,...)
{
  /* Используем прямой доступ к аргументам через указатель на char */
  char far *arg_ptr = (char far *)&format;
  char c, str[32], tmp_str[32];
  int near_ptr, offset, segment, width, precision, zero_pad;
  char far *far_ptr;
  int len, i, j, is_negative, base, idx, padding, is_long;
  char tmp_char;
  unsigned long num_long;
  unsigned int num;
  
  /* Пропускаем far pointer format (4 байта) */
  arg_ptr += 4;

  while ((c = *(format++)) != 0)
    {
      if (c != '%')
        putchar (c);
      else
        {
          /* Сброс флагов */
          width = 0;
          precision = -1;
          zero_pad = 0;
          is_long = 0;
          
          /* Парсинг формата */
          c = *format;
          
          /* Флаг 0 */
          if (c == '0')
            {
              zero_pad = 1;
              format++;
              c = *format;
            }
          
          /* Ширина */
          while (c >= '0' && c <= '9')
            {
              width = width * 10 + (c - '0');
              format++;
              c = *format;
            }
          
          /* Точность */
          if (c == '.')
            {
              format++;
              c = *format;
              precision = 0;
              while (c >= '0' && c <= '9')
                {
                  precision = precision * 10 + (c - '0');
                  format++;
                  c = *format;
                }
            }
          
          /* Модификатор 'l' для long */
          if (c == 'l')
            {
              is_long = 1;
              format++;
              c = *format;
            }
          
          /* Тип спецификатора */
          c = *(format++);
          
          switch (c)
            {
            case 'd':
            case 'x':
            case 'X':
            case 'u':
              if (is_long)
                {
                  /* 32-битное число (long) - 4 байта */
                  /* В 16-битной среде передается как два слова */
                  num_long = *(unsigned long far *)arg_ptr;
                  arg_ptr += 4;
                  
                  *convert_to_ascii_long(str, c, num_long) = 0;
                  
                  len = 0;
                  while (str[len]) len++;
                  
                  /* Применяем точность (для десятичных чисел) */
                  if (precision >= 0 && (c == 'd' || c == 'u'))
                    {
                      zero_pad = 0;
                      
                      if (precision > len)
                        {
                          padding = precision - len;
                          for (i = len; i >= 0; i--)
                            str[i + padding] = str[i];
                          for (i = 0; i < padding; i++)
                            str[i] = '0';
                          len += padding;
                        }
                      else if (precision == 0 && str[0] == '0')
                        {
                          str[0] = 0;
                          len = 0;
                        }
                    }
                  
                  /* Применяем ширину */
                  if (width > len)
                    {
                      padding = width - len;
                      if (zero_pad && precision < 0)
                        {
                          if (str[0] == '-')
                            {
                              putchar('-');
                              for (i = 0; i < padding; i++)
                                putchar('0');
                              putstr(str + 1);
                            }
                          else
                            {
                              for (i = 0; i < padding; i++)
                                putchar('0');
                              putstr(str);
                            }
                        }
                      else
                        {
                          for (i = 0; i < padding; i++)
                            putchar(' ');
                          putstr(str);
                        }
                    }
                  else
                    {
                      putstr(str);
                    }
                }
              else
                {
                  /* 16-битное число (int) - 2 байта */
                  num = *(unsigned int far *)arg_ptr;
                  arg_ptr += 2;
                  
                  *convert_to_ascii(str, c, num) = 0;
                  
                  len = 0;
                  while (str[len]) len++;
                  
                  /* Применяем точность (для десятичных чисел) */
                  if (precision >= 0 && (c == 'd' || c == 'u'))
                    {
                      zero_pad = 0;
                      
                      if (precision > len)
                        {
                          padding = precision - len;
                          for (i = len; i >= 0; i--)
                            str[i + padding] = str[i];
                          for (i = 0; i < padding; i++)
                            str[i] = '0';
                          len += padding;
                        }
                      else if (precision == 0 && str[0] == '0')
                        {
                          str[0] = 0;
                          len = 0;
                        }
                    }
                  
                  /* Применяем ширину */
                  if (width > len)
                    {
                      padding = width - len;
                      if (zero_pad && precision < 0)
                        {
                          if (str[0] == '-')
                            {
                              putchar('-');
                              for (i = 0; i < padding; i++)
                                putchar('0');
                              putstr(str + 1);
                            }
                          else
                            {
                              for (i = 0; i < padding; i++)
                                putchar('0');
                              putstr(str);
                            }
                        }
                      else
                        {
                          for (i = 0; i < padding; i++)
                            putchar(' ');
                          putstr(str);
                        }
                    }
                  else
                    {
                      putstr(str);
                    }
                }
              break;

            case 'c':
              tmp_char = *(char far *)arg_ptr;
              arg_ptr += 2; /* char расширяется до int в вызовах cdecl */
              
              if (width > 1)
                {
                  for (i = 1; i < width; i++)
                    putchar(' ');
                }
              putchar(tmp_char);
              break;

            case 's':
              {
                /* near указатель - 2 байта */
                char *str_ptr = *(char * far *)arg_ptr;
                arg_ptr += 2;
                
                len = 0;
                if (precision >= 0)
                  {
                    while (str_ptr[len] && len < precision)
                      len++;
                  }
                else
                  {
                    while (str_ptr[len])
                      len++;
                  }
                
                if (width > len)
                  {
                    for (i = len; i < width; i++)
                      putchar(' ');
                  }
                
                for (i = 0; i < len; i++)
                  putchar(str_ptr[i]);
              }
              break;

            case 'S':
              /* far указатель - 4 байта (сегмент:смещение) */
              offset = *(unsigned int far *)arg_ptr;
              arg_ptr += 2;
              segment = *(unsigned int far *)arg_ptr;
              arg_ptr += 2;
              far_ptr = MK_FP(segment, offset);
              
              len = 0;
              if (precision >= 0)
                {
                  while (far_ptr[len] && len < precision)
                    len++;
                }
              else
                {
                  while (far_ptr[len])
                    len++;
                }
              
              if (width > len)
                {
                  for (i = len; i < width; i++)
                    putchar(' ');
                }
              
              for (i = 0; i < len; i++)
                putchar(far_ptr[i]);
              break;

            case 'p': /* near pointer */
              near_ptr = *(unsigned int far *)arg_ptr;
              arg_ptr += 2;
              putstr("0x");
              *convert_to_ascii(str, 'x', near_ptr) = 0;
              len = 0;
              while (str[len]) len++;
              
              if (width > 0)
                {
                  for (i = len; i < width; i++)
                    putchar('0');
                }
              else if (precision > 0)
                {
                  for (i = len; i < precision; i++)
                    putchar('0');
                }
              else
                {
                  for (i = len; i < 4; i++)
                    putchar('0');
                }
              putstr(str);
              break;

            case 'P': /* far pointer */
            case 'F': /* Обработка %Fp */
              {
                char next_c = *format;
                if (c == 'F' && next_c == 'p')
                {
                  format++;
                }
                offset = *(unsigned int far *)arg_ptr;
                arg_ptr += 2;
                segment = *(unsigned int far *)arg_ptr;
                arg_ptr += 2;
                
                putstr("0x");
                *convert_to_ascii(str, 'x', segment) = 0;
                len = 0;
                while (str[len]) len++;
                
                if (width > 0)
                  {
                    for (i = len; i < width; i++)
                      putchar('0');
                  }
                else if (precision > 0)
                  {
                    for (i = len; i < precision; i++)
                      putchar('0');
                  }
                else
                  {
                    for (i = len; i < 4; i++)
                      putchar('0');
                  }
                putstr(str);
                
                putchar(':');
                
                *convert_to_ascii(str, 'x', offset) = 0;
                len = 0;
                while (str[len]) len++;
                
                if (width > 0)
                  {
                    for (i = len; i < width; i++)
                      putchar('0');
                  }
                else if (precision > 0)
                  {
                    for (i = len; i < precision; i++)
                      putchar('0');
                  }
                else
                  {
                    for (i = len; i < 4; i++)
                      putchar('0');
                  }
                putstr(str);
              }
              break;
              
            case '%':
              putchar('%');
              break;
              
            default:
              putchar('%');
              putchar(c);
              break;
            }
        }
    }
}

#pragma code_seg();
