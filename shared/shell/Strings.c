/*
 * 				Shell Library Functions
 *
 * Copyright 1993, 1994, 1995 Alexandre Julliard
 * Copyright 1997 Willows Software, Inc. 
 * Copyright 1998 Marcus Meissner
 * Copyright 2000 Juergen Schmied
 * Copyright 2002 Eric Pouech
 * Copyright 2023 Yuri Prokushev
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */

#include "Shell.h"

void FAR * lmemset (void FAR * dest, int val, int len)
{
  char FAR *ptr = (char FAR *)dest;
  while (len-- > 0)
    *ptr++ = val;
  return dest;
}

void lmemcpy(void FAR * s1, void FAR * s2, unsigned length)
{	char FAR * p;
	char FAR * q;

	if(length) {
		p = s1;
		q = s2;
		do *p++ = *q++;
		while(--length);
	}
}

int lstrnicmp(char FAR *s1, const char FAR *s2, int n)
{

    if (n == 0)
	return 0;
    do {
	if (AnsiUpperChar(*s1) != AnsiUpperChar(*s2++))
		return AnsiUpperChar(*(unsigned char FAR *) s1) -
		AnsiUpperChar(*(unsigned char FAR *) --s2);
	if (*s1++ == 0)
		break;
	} while (--n != 0);
	return 0;
}

char FAR *lstrchr(const char FAR *s, int c)
{
	const char ch = c;

	for ( ; *s != ch; s++)
		if (*s == '\0')
			return 0;
	return (char FAR *)s;
}
