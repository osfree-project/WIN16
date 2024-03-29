%{
/*
 * Help Viewer
 *
 * Copyright 1996 Ulrich Schmid
 * Copyright 2002 Eric Pouech
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
%}
%x quote
%{
#include <assert.h>
#include "macro.h"

#define GlobalPtrHandle(lp) \
  ((HGLOBAL)LOWORD(GlobalHandle(SELECTOROF(lp))))

#define     GlobalUnlockPtr(lp)      \
                GlobalUnlock(GlobalPtrHandle(lp))

#define GlobalFreePtr(lp) \
  (GlobalUnlockPtr(lp),(BOOL)GlobalFree(GlobalPtrHandle(lp)))

#define GlobalAllocPtr(flags, cb) \
  (GlobalLock(GlobalAlloc((flags), (cb))))

//#include "wine/debug.h"

//WINE_DEFAULT_DEBUG_CHANNEL(winhelp);

static LPCSTR  macroptr;
static LPSTR   strptr;
static int     quote_stack[32];
static int     quote_stk_idx = 0;
struct lexret  yylval;

#define YY_INPUT(buf,result,max_size)\
  if ((result = *macroptr ? 1 : 0)) buf[0] = *macroptr++;

#define YY_NO_UNPUT
%}
%%

[-+]?[0-9]+             yylval.integer = strtol(yytext, NULL, 10);	return INTEGER;
[-+]?0[xX][0-9a-f]+	yylval.integer = strtol(yytext, NULL, 16);	return INTEGER;

[a-zA-Z][_0-9a-zA-Z]*   return MACRO_Lookup(yytext, &yylval);

\`	    |
\"	    |
\'          |
<quote>\`   |
<quote>\"   |
<quote>\'   {
    if (quote_stk_idx == 0 ||
        (yytext[0] == '\"' && quote_stack[quote_stk_idx - 1] != '\"') ||
        (yytext[0] == '`'))
    {
        /* opening a new one */
        if (quote_stk_idx == 0)
        {
            strptr = GlobalAllocPtr(GPTR, lstrlen(macroptr) + 1);
            yylval.string = strptr;
            BEGIN(quote);
        }
        else *strptr++ = yytext[0];
        quote_stack[quote_stk_idx++] = yytext[0];
        assert(quote_stk_idx < sizeof(quote_stack) / sizeof(quote_stack[0]));
    }
    else
    {
        if (yytext[0] == '`') assert(0);
        /* close the current quote */
        if (--quote_stk_idx == 0)
        {
            BEGIN INITIAL;
            *strptr++ = '\0';
            return STRING;
        }
        else *strptr++ = yytext[0];
    }
}

<quote>.                *strptr++ = yytext[0];
<quote>\\.	        *strptr++ = yytext[1];
<quote><<EOF>>	        return 0;

" "
.			return yytext[0];
%%

#if 0
/* all code for testing macros */
#include "winhelp.h"
static CHAR szTestMacro[256];

static LRESULT CALLBACK MACRO_TestDialogProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (msg == WM_COMMAND && wParam == IDOK)
    {
        GetDlgItemText(hDlg, 99, szTestMacro, sizeof(szTestMacro));
        EndDialog(hDlg, IDOK);
        return TRUE;
    }
    return FALSE;
}

void macro_test(void)
{
    WNDPROC lpfnDlg = MakeProcInstance(MACRO_TestDialogProc, Globals.hInstance);
    DialogBox(Globals.hInstance, STRING_DIALOG_TEST, Globals.active_win->hMainWnd, (DLGPROC)lpfnDlg);
    FreeProcInstance(lpfnDlg);
    macro = szTestMacro;
}
#endif

/* small helper function for debug messages */
static const char* ts(int t)
{
    static char c[2] = {0,0};

    switch (t)
    {
    case EMPTY: return "EMPTY";
    case VOID_FUNCTION: return "VOID_FUNCTION";
    case BOOL_FUNCTION: return "BOOL_FUNCTION";
    case INTEGER: return "INTEGER";
    case STRING: return "STRING";
    case IDENTIFIER: return "IDENTIFIER";
    default: c[0] = (char)t; return c;
    }
}

static int MACRO_CallBoolFunc(FARPROC fn, const char far * args, void ** ret);

/******************************************************************
 *		MACRO_CheckArgs
 *
 * checks number of arguments against prototype, and stores arguments on
 * stack pa for later call
 * returns -1 on error, otherwise the number of pushed parameters
 */
static int MACRO_CheckArgs(void* pa[], unsigned max, const char far * args)
{
    int         t;
    int         idx = 0;

    //WINE_TRACE("Checking %s\n", args);

    if (yylex() != '(') {//WINE_WARN("missing (\n");
		return -1;
	}

    if (*args)
    {
        for (;;)
        {
            t = yylex();
            //WINE_TRACE("Got %s <=> %c\n", ts(t), *args);

            switch (*args)
            {
            case 'S': 
                if (t != STRING)
                {//WINE_WARN("missing S\n");
					return -1;
				}
                pa[idx] = (void*)yylval.string;  
                break;
            case 'U':
            case 'I':
                if (t != INTEGER)
                {
					//WINE_WARN("missing U\n");
					return -1;
				}   
                pa[idx] = (void*)yylval.integer; 
                break;
            case 'B':
                if (t != BOOL_FUNCTION) 
                {//WINE_WARN("missing B\n");
					return -1;
				}   
                if (MACRO_CallBoolFunc(yylval.function, yylval.proto, &pa[idx]) == 0)
                    return -1;
                break;
            default: 
                //WINE_WARN("unexpected %s while args is %c\n", ts(t), *args);
                return -1;
            }
            idx++;
            if (*++args == '\0') break;
            if (yylex() != ',') {//WINE_WARN("missing ,\n");
				return -1;
			}
            if (idx == max) {
				//WINE_FIXME("stack overflow (%d)\n", max);
				return -1;
			}
        }
    }
    if (yylex() != ')') {
		//WINE_WARN("missing )\n");return -1;
	}
    return idx;
}

/******************************************************************
 *		MACRO_CallBoolFunc
 *
 * Invokes boolean function fn, which arguments are defined by args
 * stores bool result into ret
 */
static int MACRO_CallBoolFunc(FARPROC fn, const char far * args, void** ret)
{
    void *       pa[2];
    int         idx = MACRO_CheckArgs(pa, sizeof(pa)/sizeof(pa[0]), args);

    if (idx == -1) return 0;
    if (!fn)       return 1;

    //WINE_TRACE("calling with %u pmts\n", idx);

    switch (idx)
    {
    case 0: *ret = (void*)(fn)();          break;
    case 1: *ret = (void*)(fn)(pa[0]);     break;
    default: {}//WINE_FIXME("NIY\n");
    }

    return 1;
}

/******************************************************************
 *		MACRO_CallVoidFunc
 *
 *
 */
static int MACRO_CallVoidFunc(FARPROC fn, const char far * args)
{
    void*       pa[6];
    int         idx = MACRO_CheckArgs(pa, sizeof(pa)/sizeof(pa[0]), args);

    if (idx == -1) return 0;
    if (!fn)       return 1;

    //WINE_TRACE("calling %p with %u pmts\n", fn, idx);

    switch (idx)
    {
    case 0: (fn)();                                     break;
    case 1: (fn)(pa[0]);                                break;
    case 2: (fn)(pa[0],pa[1]);                          break;
    case 3: (fn)(pa[0],pa[1],pa[2]);                    break;
    case 4: (fn)(pa[0],pa[1],pa[2],pa[3]);              break;
    case 5: (fn)(pa[0],pa[1],pa[2],pa[3],pa[4]);        break;
    case 6: (fn)(pa[0],pa[1],pa[2],pa[3],pa[4],pa[5]);  break;
    default: {}//WINE_FIXME("NIY\n");
    }

    return 1;
}

BOOL MACRO_ExecuteMacro(LPCSTR macro)
{
    int t;

    //WINE_TRACE("%s\n", wine_dbgstr_a(macro));

    macroptr = macro;

    while ((t = yylex()) != EMPTY)
    {
        switch (t)
        {
        case VOID_FUNCTION:
            //WINE_TRACE("got type void func(%s)\n", yylval.proto);
            MACRO_CallVoidFunc(yylval.function, yylval.proto);
            break;
        case BOOL_FUNCTION:
            //WINE_WARN("got type bool func(%s)\n", yylval.proto);
            break;
        default:
            //WINE_WARN("got unexpected type %s\n", ts(t));
            return 0;
        }
        switch (t = yylex())
        {
        case EMPTY:     return 1;
        case ';':       break;
        default:        return 0;
        }
    }

    if (strptr)
    {
        GlobalFreePtr(strptr);
        strptr = NULL;
    }
    quote_stk_idx = 0;

    return 1;
}

#ifndef yywrap
int yywrap(void) { return 1; }
#endif
