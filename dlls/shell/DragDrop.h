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

/* Messages related to drag/drop feature */

#ifndef	WM_DROPOBJECT
#define	WM_DROPOBJECT		0x022a
#endif

#ifndef	WM_QUERYDROPOBJECT
#define	WM_QUERYDROPOBJECT	0x022b
#endif

#ifndef	WM_BEGINDRAG
#define	WM_BEGINDRAG		0x022c
#endif

#ifndef	WM_DRAGLOOP
#define	WM_DRAGLOOP		0x022d
#endif

#ifndef	WM_DRAGSELECT
#define	WM_DRAGSELECT		0x022e
#endif

#ifndef	WM_DRAGMOVE
#define	WM_DRAGMOVE		0x022e
#endif

typedef struct tagDROPFILESTRUCT {
    WORD	wSize;
    POINT	ptMousePos;
    BOOL	fInNonClientArea;
} DROPFILESTRUCT, FAR *LPDROPFILESTRUCT;

