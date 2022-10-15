int toupper (int c);

int strnicmp(char far *s1, const char far *s2, int n)
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
