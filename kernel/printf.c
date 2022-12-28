/*
 *   mini-libc functions
 *
 */

/* This function prints one char */
extern  void putchar(char);
#pragma aux putchar               = \
        "mov ah, 2"          \
        "int 21h"          \
        parm                   [dl];

char * convert_to_ascii (char *buf, int c,...)
{
  unsigned long num = *((&c) + 1), mult = 10;
  char *ptr = buf;

  if (c == 'x' || c == 'X')
    mult = 16;

  if ((num & 0x80000000uL) && c == 'd')
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

extern void _cdecl printf (const char *format,...);

void printf (const char *format,...)
{
  int *dataptr = (int *) &format;
  char c, str[16];

  dataptr++;

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
            *convert_to_ascii (str, c, *((unsigned long *) dataptr++)) = 0;
            putstr (str);
            break;

          case 'c':
            putchar ((*(dataptr++)) & 0xff);
            break;

          case 's':
            putstr ((char *) *(dataptr++));
            break;
          }
    }
}
