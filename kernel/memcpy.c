void memcpy(void far * s1, void far * s2, unsigned length)
{	char far * p;
	char far * q;

	if(length) {
		p = s1;
		q = s2;
		do *p++ = *q++;
		while(--length);
	}
}
