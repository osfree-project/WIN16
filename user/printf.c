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
  char c, str[16];
  int near_ptr, offset, segment;
  char far *far_ptr;
  int len, i;

  dataptr+=2;

  while ((c = *(format++)) != 0)
    {
      if (c != '%')
        putchar (c);
      else
        switch (c = *(format++))
          {
          case 'd':
          case 'x':
          case 'X':
          case 'u':
            *convert_to_ascii (str, c, *((unsigned int *) dataptr++)) = 0;
            putstr (str);
            break;

          case 'c':
            putchar ((*(dataptr++)) & 0xff);
            break;

          case 's':
            putstr ((char *) *(dataptr++));
            break;

          case 'S':
            offset = *(dataptr++);
            segment = *(dataptr++);
            far_ptr = (char far *)MK_FP(segment, offset);
            putstrfar (far_ptr);           
            break;

          case 'p': /* near pointer */
            near_ptr = *(dataptr++);
            putstr("0x");
            *convert_to_ascii(str, 'x', near_ptr) = 0;
            len = 0;
            while (str[len]) len++;
            for (i = len; i < 4; i++) {
                putchar('0');
            }
            putstr(str);
            break;

          case 'P': /* far pointer */
            offset = *(dataptr++);
            segment = *(dataptr++);
            
            putstr("0x");
            *convert_to_ascii(str, 'x', segment) = 0;
            len = 0;
            while (str[len]) len++;
            for (i = len; i < 4; i++) {
                putchar('0');
            }
            putstr(str);
            
            putchar(':');
            
            *convert_to_ascii(str, 'x', offset) = 0;
            len = 0;
            while (str[len]) len++;
            for (i = len; i < 4; i++) {
                putchar('0');
            }
            putstr(str);
            break;
          }
    }
}
