void far * memset (void far *start, int c, int len)
{
  char far *p = start;

  while (len -- > 0)
    *p ++ = c;

  return start;
}
