/*
 *   mini-libc functions
 *
 */

#ifndef MK_FP
#define MK_FP(seg,off) ((void far *)(((unsigned long)(seg) << 16) | (unsigned)(off)))
#endif

/* This function prints one char */
extern  void putchar(char);
#pragma aux putchar               = \
        "mov ah, 2"          \
        "int 21h"          \
        parm                   [dl];

char * convert_to_ascii (char *buf, int c,...)
{
  unsigned int num = *((&c) + 1), mult = 10;
  char *ptr = buf;

  if (c == 'x' || c == 'X')
    mult = 16;

  if ((num & 0x8000) && c == 'd')
    {
      num = (~num) + 1;
      *(ptr++) = '-';
      buf++;
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

  return ptr;
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

void _cdecl printf (char far *format,...)
{
  int *dataptr = (int *) &format;
  char c, str[32], tmp_str[32];
  int near_ptr, offset, segment, width, precision, zero_pad;
  char far *far_ptr;
  int len, i, j, num, is_negative, base, idx, padding;
  char tmp_char;
  
  dataptr+=2;

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
          
          /* Тип спецификатора */
          c = *(format++);
          
          switch (c)
            {
            case 'd':
            case 'x':
            case 'X':
            case 'u':
              num = *((unsigned int *) dataptr++);
              base = (c == 'x' || c == 'X') ? 16 : 10;
              is_negative = 0;
              
              if ((int)num < 0 && c == 'd')
                {
                  is_negative = 1;
                  num = -(int)num;
                }
              
              /* Преобразование числа в строку */
              idx = 0;
              if (num == 0)
                {
                  if (precision != 0)
                    tmp_str[idx++] = '0';
                }
              else
                {
                  while (num != 0)
                    {
                      int digit = num % base;
                      if (digit < 10)
                        tmp_str[idx++] = '0' + digit;
                      else if (c == 'X')
                        tmp_str[idx++] = 'A' + digit - 10;
                      else
                        tmp_str[idx++] = 'a' + digit - 10;
                      num /= base;
                    }
                }
              tmp_str[idx] = 0;
              
              /* Переворачиваем строку */
              for (i = 0; i < idx / 2; i++)
                {
                  tmp_char = tmp_str[i];
                  tmp_str[i] = tmp_str[idx - i - 1];
                  tmp_str[idx - i - 1] = tmp_char;
                }
              
              /* Применяем точность */
              if (precision >= 0)
                {
                  /* Точность переопределяет zero_pad */
                  zero_pad = 0;
                  
                  len = 0;
                  while (tmp_str[len]) len++;
                  
                  if (precision > len)
                    {
                      /* Дополняем нулями слева */
                      for (i = precision - 1; i >= 0; i--)
                        {
                          if (i >= precision - len)
                            str[i] = tmp_str[i - (precision - len)];
                          else
                            str[i] = '0';
                        }
                      str[precision] = 0;
                    }
                  else if (precision == 0 && tmp_str[0] == '0')
                    {
                      /* Специальный случай: точность 0 для нуля */
                      str[0] = 0;
                    }
                  else
                    {
                      /* Копируем как есть */
                      i = 0;
                      while (tmp_str[i])
                        {
                          str[i] = tmp_str[i];
                          i++;
                        }
                      str[i] = 0;
                    }
                }
              else
                {
                  /* Без точности - просто копируем */
                  i = 0;
                  while (tmp_str[i])
                    {
                      str[i] = tmp_str[i];
                      i++;
                    }
                  str[i] = 0;
                }
              
              /* Добавляем знак минус для отрицательных чисел */
              len = 0;
              while (str[len]) len++;
              
              if (is_negative)
                {
                  /* Сдвигаем строку вправо и добавляем минус */
                  for (i = len; i >= 0; i--)
                    str[i + 1] = str[i];
                  str[0] = '-';
                  len++;
                }
              
              /* Применяем ширину */
              if (width > len)
                {
                  padding = width - len;
                  if (zero_pad && precision < 0)
                    {
                      /* Заполнение нулями */
                      if (is_negative)
                        {
                          /* Для отрицательных: минус, затем нули */
                          putchar('-');
                          for (i = 0; i < padding; i++)
                            putchar('0');
                          /* Пропускаем минус в строке */
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
                      /* Заполнение пробелами слева */
                      for (i = 0; i < padding; i++)
                        putchar(' ');
                      putstr(str);
                    }
                }
              else
                {
                  putstr(str);
                }
              break;

            case 'c':
              tmp_char = (*(dataptr++)) & 0xff;
              
              /* Ширина для символа */
              if (width > 1)
                {
                  for (i = 1; i < width; i++)
                    putchar(' ');
                }
              putchar(tmp_char);
              break;

            case 's':
              {
                char *str_ptr = (char *) *(dataptr++);
                
                /* Определяем длину строки с учетом точности */
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
                
                /* Применяем ширину */
                if (width > len)
                  {
                    for (i = len; i < width; i++)
                      putchar(' ');
                  }
                
                /* Выводим строку */
                for (i = 0; i < len; i++)
                  putchar(str_ptr[i]);
              }
              break;

            case 'S':
              offset = *(dataptr++);
              segment = *(dataptr++);
              far_ptr = (char far *)MK_FP(segment, offset);
              
              /* Определяем длину far-строки с учетом точности */
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
              
              /* Применяем ширину */
              if (width > len)
                {
                  for (i = len; i < width; i++)
                    putchar(' ');
                }
              
              /* Выводим строку */
              for (i = 0; i < len; i++)
                putchar(far_ptr[i]);
              break;

            case 'p': /* near pointer */
              near_ptr = *(dataptr++);
              putstr("0x");
              *convert_to_ascii(str, 'x', near_ptr) = 0;
              len = 0;
              while (str[len]) len++;
              
              /* Применяем ширину с заполнением нулями */
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
                  /* По умолчанию 4 цифры для указателей */
                  for (i = len; i < 4; i++)
                    putchar('0');
                }
              putstr(str);
              break;

            case 'P': /* far pointer */
              offset = *(dataptr++);
              segment = *(dataptr++);
              
              putstr("0x");
              /* Печатаем сегмент */
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
              
              /* Печатаем смещение */
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
              break;
            }
        }
    }
}
