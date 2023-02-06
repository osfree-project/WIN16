void lmemcpy(void far * s1, void far * s2, unsigned length)
{	char far * p;
	char far * q;

	if(length) {
		p = s1;
		q = s2;
		do *p++ = *q++;
		while(--length);
	}
}

int toupper (int c)
{
  if (c >= 'a' && c <= 'z')
    return (c + ('A' - 'a'));

  return c;
}

int lstrnicmp(char far *s1, const char far *s2, int n)
{

    if (n == 0)
	return 0;
    do {
	if (toupper(*s1) != toupper(*s2++))
	    return toupper(*(unsigned char *) s1) -
		toupper(*(unsigned char *) --s2);
	if (*s1++ == 0)
	    break;
    } while (--n != 0);
    return 0;
}
