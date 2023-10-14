/************************************************************************
  T h e   O p e n   F o u n d a t i o n    C l a s s e s
 ------------------------------------------------------------------------
  Filename   : TestCString.cpp
  Version    : 0.20
  Author(s)  : William D. Herndon

 --[ Description ]-------------------------------------------------------

  This file is part of the OFC - Open Foundation Classes -.
  It is MFC compatible.
  Test program for OFC.

 --[ History ] ----------------------------------------------------------

cb  = Carsten Breuer     (Carsten.Breuer@breuer-software.de)
gv  = Geurt Vos          (geurt@users.sourceforge.net)
id  = Ivan Deras         (ideras@users.sourceforge.net)
tm  = Tim Musschoot      (timussch@users.sourceforge.net)
wdh = William D. Herndon (shadowdog@users.sourceforge.net)

mm-dd-yy  ver   who  what
10-26-03  0.10  wdh  Created from cstring_test.cpp
05-30-04  0.20  wdh  Changed OfcTest.h to StdAfx.h

 --[ How to compile ]----------------------------------------------------

  This file was developed under DevCpp, free software from
  Bloodshed Software, http://www.bloodshed.net/

 --[ Where to get help/information ]-------------------------------------

  The author              : shadowdog@users.sourceforge.net

 --[ License ] ----------------------------------------------------------

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

 ------------------------------------------------------------------------
  Copyright (c) 2000-04 The Open Foundation Classes
  Copyright (c) 2003-04 William D. Herndon
/************************************************************************/
#include "StdAfx.h"
#include "resource.h"
#include <oleauto.h>

void dbg_printf(LPCSTR pszForm,...);

    UCHAR g_szTestString[] = {
        'T', 'e', 's', 't', ':', ' ',
        0xE4/*ae*/, 0xF6/*oe*/, 0xFC/*ue*/,
        0xC4/*AE*/, 0xD6/*OE*/, 0xDC/*UE*/,
        0xDF/*doubles (eszet)*/,
        0xE0/*agrave*/, 0xE1/*aaccent*/, 0xE2/*acirc*/,
        0xE8/*egrave*/, 0xE9/*eaccent*/, 0xEA/*ecirc*/,
        0xEC/*igrave*/, 0xED/*iaccent*/, 0xEE/*icirc*/,
        0xF2/*ograve*/, 0xF3/*oaccent*/, 0xF4/*ocirc*/,
        0xF9/*ugrave*/, 0xFA/*uaccent*/, 0xFB/*ucirc*/,
        0
    };

    UCHAR g_szTestStringOem[] = {
        'T', 'e', 's', 't', ':', ' ',
        0x84/*ae*/, 0x94/*oe*/, 0x81/*ue*/,
        0x8E/*AE*/, 0x99/*OE*/, 0x9A/*UE*/,
        0xE1/*doubles (eszet)*/,
        0x85/*agrave*/, 0xA0/*aaccent*/, 0x83/*acirc*/,
        0x8A/*egrave*/, 0x82/*eaccent*/, 0x88/*ecirc*/,
        0x8D/*igrave*/, 0xA1/*iaccent*/, 0x8C/*icirc*/,
        0x95/*ograve*/, 0xA2/*oaccent*/, 0x93/*ocirc*/,
        0x97/*ograve*/, 0xA3/*oaccent*/, 0x96/*ocirc*/,
        0
    };

void TestCString()
{
    CString str1;
    CString str2("String");
    CString str3('A', 10);
    CString str4("Abcdef", 3);
    CString str5( str2 );
    BOOL bResult, bAllResult = TRUE, bMsCompat = TRUE;

    outPrintf("\r\n"
        "Testing CString functionality\r\n"
        "-----------------------------\r\n");

    // Show all the different kinds of initialization
    outPrintf("\r\nInitialization / Assignment\r\n");

    bResult = (strcmp(str1, "") == 0);
    bMsCompat &= bResult;
    bAllResult &= bResult;
    outPrintf("CString str1; str1 => \"%s\" = %s\r\n",
        (LPCSTR)str1, ResultStr(bResult));

    bResult = (strcmp(str2, "String") == 0);
    bMsCompat &= bResult;
    bAllResult &= bResult;
    outPrintf("CString str2(\"String\"); str2 => \"%s\" = %s\r\n",
        (LPCSTR)str2, ResultStr(bResult));

    bResult = (strcmp(str3, "AAAAAAAAAA") == 0);
    bMsCompat &= bResult;
    bAllResult &= bResult;
    outPrintf("CString str3('A', 10); str3 => \"%s\" = %s\r\n",
        (LPCSTR)str3, ResultStr(bResult));

    bResult = (strcmp(str4, "Abc") == 0);
    bMsCompat &= bResult;
    bAllResult &= bResult;
    outPrintf("CString str4(\"Abcdef\", 3); str4 => \"%s\" = %s\r\n",
        (LPCSTR)str4, ResultStr(bResult));

    bResult = (strcmp(str5, "String") == 0);
    bMsCompat &= bResult;
    bAllResult &= bResult;
    outPrintf("CString str5(str2); str5 => \"%s\" = %s\r\n",
        (LPCSTR)str4, ResultStr(bResult));

    str1 = str4;
    bResult = (strcmp(str1, "Abc") == 0);
    bMsCompat &= bResult;
    bAllResult &= bResult;
    outPrintf("str1 = str4; str1=>\"%s\" = %s\r\n",
        (LPCSTR)str1, ResultStr(bResult));

    str1 = 'Z';
    bResult = (strcmp(str1, "Z") == 0);
    bMsCompat &= bResult;
    bAllResult &= bResult;
    outPrintf("str1 = 'Z'; str1=>\"%s\" = %s\r\n",
        (LPCSTR)str1, ResultStr(bResult));

    str1 = (const unsigned char*)"xyz";
    bResult = (strcmp(str1, "xyz") == 0);
    bMsCompat &= bResult;
    bAllResult &= bResult;
    outPrintf("str1 = (const unsigned char*)\"xyz\"; str1=>\"%s\" = %s\r\n",
        (LPCSTR)str1, ResultStr(bResult));

    str1 = (LPCSTR)"pqr";
    bResult = (strcmp(str1, "pqr") == 0);
    bMsCompat &= bResult;
    bAllResult &= bResult;
    outPrintf("str1 = (LPCSTR)\"pqr\"; str1=>\"%s\" = %s\r\n",
        (LPCSTR)str1, ResultStr(bResult));

    // Show the element and addition operators
    outPrintf("\r\nOperators: [] + +=\r\n");
    bResult = ((str4[0] == 'A') && (str4[1] == 'b') && (str4[2] == 'c'));
    bMsCompat &= bResult;
    bAllResult &= bResult;
    outPrintf("str4[0]:'%c',str4[1]:'%c',str4[2]:'%c' => %s\r\n",
        str4[0], str4[1], str4[2], ResultStr(bResult));

    str1 = str2 + str3;
    bResult = (strcmp(str1, "StringAAAAAAAAAA") == 0);
    bMsCompat &= bResult;
    bAllResult &= bResult;
    outPrintf("str1 = str2 + str3; str1=>\"%s\" = %s\r\n",
        (LPCSTR)str1, ResultStr(bResult));

    str1 = str2 + 'Z';
    bResult = (strcmp(str1, "StringZ") == 0);
    bMsCompat &= bResult;
    bAllResult &= bResult;
    outPrintf("str1 = str2 + 'Z'; str1=>\"%s\" = %s\r\n",
        (LPCSTR)str1, ResultStr(bResult));

    str1 = 'Q' + str2;
    bResult = (strcmp(str1, "QString") == 0);
    bMsCompat &= bResult;
    bAllResult &= bResult;
    outPrintf("str1 = 'Q' + str2; str1=>\"%s\" = %s\r\n",
        (LPCSTR)str1, ResultStr(bResult));

    str1 = str2 + "XYZ";
    bResult = (strcmp(str1, "StringXYZ") == 0);
    bMsCompat &= bResult;
    bAllResult &= bResult;
    outPrintf("str1 = str2 + \"XYZ\"; str1=>\"%s\" = %s\r\n",
        (LPCSTR)str1, ResultStr(bResult));

    str1 = "ABC" + str2;
    bResult = (strcmp(str1, "ABCString") == 0);
    bMsCompat &= bResult;
    bAllResult &= bResult;
    outPrintf("str1 = \"ABC\" + str2; str1=>\"%s\" = %s\r\n",
        (LPCSTR)str1, ResultStr(bResult));

    str1 += str2;
    bResult = (strcmp(str1, "ABCStringString") == 0);
    bMsCompat &= bResult;
    bAllResult &= bResult;
    outPrintf("str1 += str2; str1=>\"%s\" = %s\r\n",
        (LPCSTR)str1, ResultStr(bResult));

    str1 += 'W';
    bResult = (strcmp(str1, "ABCStringStringW") == 0);
    bMsCompat &= bResult;
    bAllResult &= bResult;
    outPrintf("str1 += 'W'; str1=>\"%s\" = %s\r\n",
        (LPCSTR)str1, ResultStr(bResult));

    str1 += "XYZ";
    bResult = (strcmp(str1, "ABCStringStringWXYZ") == 0);
    bMsCompat &= bResult;
    bAllResult &= bResult;
    outPrintf("str1 += \"XYZ\"; str1=>\"%s\" = %s\r\n",
        (LPCSTR)str1, ResultStr(bResult));

    // Miscellaneous methods
    outPrintf("\r\nMiscellaneous methods\r\n");

    int iTmp1, iTmp2, iTmp3;
    iTmp1 = str2.GetLength();
    bResult = (iTmp1 == (int)strlen(str2));
    bMsCompat &= bResult;
    bAllResult &= bResult;
    outPrintf("str2.GetLength()=>%d = %s\r\n",
        iTmp1, ResultStr(bResult));

    str1.Empty();
    bResult = ((str1.GetLength() == 0) && (strcmp(str1, "") == 0));
    bMsCompat &= bResult;
    bAllResult &= bResult;
    outPrintf("str1.Empty(); str1=>\"%s\" = %s\r\n",
        (LPCSTR)str1, ResultStr(bResult));

    BOOL bTmp1, bTmp2, bTmp3, bTmp4, bTmp5, bTmp6;
    bTmp1 = str1.IsEmpty();
    bTmp2 = str2.IsEmpty();
    bResult = ((bTmp1 == TRUE) && (bTmp2 == FALSE));
    bMsCompat &= bResult;
    bAllResult &= bResult;
    outPrintf("str1.IsEmpty()=>%d str2.IsEmpty()=>%d = %s\r\n",
        bTmp1, bTmp2, ResultStr(bResult));

    char chTmp;
    chTmp = str2.GetAt(5);
    bResult = (chTmp == 'g');
    bMsCompat &= bResult;
    bAllResult &= bResult;
    outPrintf("str2.GetAt(5)=>'%c' = %s\r\n",
        chTmp, ResultStr(bResult));

    str2.SetAt(5, 'k');
    bResult = (strcmp(str2, "Strink") == 0);
    bMsCompat &= bResult;
    bAllResult &= bResult;
    outPrintf("str2.SetAt(5, 'k')=>\"%s\" = %s\r\n",
        (LPCSTR)str2, ResultStr(bResult));

#ifndef MSVC32 // OFC extension
    str1.Fill('A', 5);
    bResult = (strcmp(str1, "AAAAA") == 0);
    bMsCompat &= bResult;
    bAllResult &= bResult;
    outPrintf("str1.Fill('A', 5)=>\"%s\" = %s\r\n",
        (LPCSTR)str1, ResultStr(bResult));
#else
    outPrintf("CString::Fill() not supported by original MFC\r\n");
    bAllResult = FALSE;
#endif

    // Comparison methods
    outPrintf("\r\nComparison methods\r\n");

    iTmp1 = str2.Compare("Strink");
    iTmp2 = str2.Compare("strink");
    iTmp3 = str2.Compare("String");
    bResult = ((iTmp1 == 0) && (iTmp2 == -1) && (iTmp3 == 1));
    bMsCompat &= bResult;
    bAllResult &= bResult;
    outPrintf("str2.Compare(\"Strink\")=>%d\r\n"
        "str2.Compare(\"strink\")=>%d\r\n"
        "str2.Compare(\"String\")=>%d =  %s\r\n",
        iTmp1, iTmp2, iTmp3, ResultStr(bResult));

    iTmp1 = str2.CompareNoCase("Strink");
    iTmp2 = str2.CompareNoCase("strinK");
    iTmp3 = str2.CompareNoCase("StrinG");
    bResult = ((iTmp1 == 0) && (iTmp2 == 0) && (iTmp3 == 1));
    bMsCompat &= bResult;
    bAllResult &= bResult;
    outPrintf("str2.CompareNoCase(\"Strink\")=>%d\r\n"
        "str2.CompareNoCase(\"strinK\")=>%d\r\n"
        "str2.CompareNoCase(\"StrinG\")=>%d = %s\r\n",
        iTmp1, iTmp2, iTmp3, ResultStr(bResult));

#if 0 // This is locale specific, but even so
	// both here and in the original MFC it
	// sometimes produces different results than strcmp (stricmp)
	// for strings only containing standard letters.
	// This contradicts the documentation for strcoll().
    iTmp1 = str2.Collate("Strink");
    iTmp2 = str2.Collate("strink");
    iTmp3 = str2.Collate("Stránk");
    bResult = ((iTmp1 == 0) && (iTmp2 == -1) && (iTmp3 == 1));
    bMsCompat &= bResult;
    bAllResult &= bResult;
    outPrintf("str2.Collate(\"Strink\")=>%d\r\n"
        "str2.Collate(\"strink\")=>%d\r\n"
        "str2.Collate(\"Stránk\")=>%d =  %s\r\n",
        iTmp1, iTmp2, iTmp3, ResultStr(bResult));

    iTmp1 = str2.CollateNoCase("Strink");
    iTmp2 = str2.CollateNoCase("strinK");
    iTmp3 = str2.CollateNoCase("Stránk");
    bResult = ((iTmp1 == 0) && (iTmp2 == 0) && (iTmp3 == 1));
    bMsCompat &= bResult;
    bAllResult &= bResult;
    outPrintf("str2.CollateNoCase(\"Strink\")=>%d\r\n"
        "str2.CollateNoCase(\"strink\")=>%d\r\n"
        "str2.CollateNoCase(\"Stránk\")=>%d =  %s\r\n",
        iTmp1, iTmp2, iTmp3, ResultStr(bResult));
#endif

    // Substring methods
    outPrintf("\r\nSubstring methods\r\n");

    str1 = str2.Mid(0);
    bResult = (strcmp(str1, "Strink") == 0);
    bMsCompat &= bResult;
    bAllResult &= bResult;
    outPrintf("str2.Mid(0)=>\"%s\" = %s\r\n",
        (LPCSTR)str1, ResultStr(bResult));

    str1 = str2.Mid(3);
    bResult = (strcmp(str1, "ink") == 0);
    bMsCompat &= bResult;
    bAllResult &= bResult;
    outPrintf("str2.Mid(3)=>\"%s\" = %s\r\n",
        (LPCSTR)str1, ResultStr(bResult));

    str1 = str2.Mid(5);
    bResult = (strcmp(str1, "k") == 0);
    bMsCompat &= bResult;
    bAllResult &= bResult;
    outPrintf("str2.Mid(5)=>\"%s\" = %s\r\n",
        (LPCSTR)str1, ResultStr(bResult));

    // MFC Allows up to 0 length remaining
    str1 = str2.Mid(6);
    bResult = (strcmp(str1, "") == 0);
    bMsCompat &= bResult;
    bAllResult &= bResult;
    outPrintf("str2.Mid(6)=>\"%s\" = %s\r\n",
        (LPCSTR)str1, ResultStr(bResult));

    str1 = str2.Mid(0, 3);
    bResult = (strcmp(str1, "Str") == 0);
    bMsCompat &= bResult;
    bAllResult &= bResult;
    outPrintf("str2.Mid(0, 3)=>\"%s\" = %s\r\n",
        (LPCSTR)str1, ResultStr(bResult));

    str1 = str2.Mid(3, 2);
    bResult = (strcmp(str1, "in") == 0);
    bMsCompat &= bResult;
    bAllResult &= bResult;
    outPrintf("str2.Mid(3, 2)=>\"%s\" = %s\r\n",
        (LPCSTR)str1, ResultStr(bResult));

    str1 = str2.Mid(3, 0);
    bResult = (strcmp(str1, "") == 0);
    bMsCompat &= bResult;
    bAllResult &= bResult;
    outPrintf("str2.Mid(3, 0)=>\"%s\" = %s\r\n",
        (LPCSTR)str1, ResultStr(bResult));

    str1 = str2.Mid(5, 3);
    bResult = (strcmp(str1, "k") == 0);
    bMsCompat &= bResult;
    bAllResult &= bResult;
    outPrintf("str2.Mid(5, 3)=>\"%s\" = %s\r\n",
        (LPCSTR)str1, ResultStr(bResult));

    // MFC Allows up to 0 length remaining
    str1 = str2.Mid(6, 3);
    bResult = (strcmp(str1, "") == 0);
    bMsCompat &= bResult;
    bAllResult &= bResult;
    outPrintf("str2.Mid(6, 3)=>\"%s\" = %s\r\n",
        (LPCSTR)str1, ResultStr(bResult));

    str1 = str2.Left(0);
    bResult = (strcmp(str1, "") == 0);
    bMsCompat &= bResult;
    bAllResult &= bResult;
    outPrintf("str2.Left(0)=>\"%s\" = %s\r\n",
        (LPCSTR)str1, ResultStr(bResult));

    str1 = str2.Left(3);
    bResult = (strcmp(str1, "Str") == 0);
    bMsCompat &= bResult;
    bAllResult &= bResult;
    outPrintf("str2.Left(3)=>\"%s\" = %s\r\n",
        (LPCSTR)str1, ResultStr(bResult));

    str1 = str2.Left(6);
    bResult = (strcmp(str1, "Strink") == 0);
    bMsCompat &= bResult;
    bAllResult &= bResult;
    outPrintf("str2.Left(6)=>\"%s\" = %s\r\n",
        (LPCSTR)str1, ResultStr(bResult));

    str1 = str2.Left(7);
    bResult = (strcmp(str1, "Strink") == 0);
    bMsCompat &= bResult;
    bAllResult &= bResult;
    outPrintf("str2.Left(7)=>\"%s\" = %s\r\n",
        (LPCSTR)str1, ResultStr(bResult));

    str1 = str2.Right(0);
    bResult = (strcmp(str1, "") == 0);
    bMsCompat &= bResult;
    bAllResult &= bResult;
    outPrintf("str2.Right(0)=>\"%s\" = %s\r\n",
        (LPCSTR)str1, ResultStr(bResult));

    str1 = str2.Right(3);
    bResult = (strcmp(str1, "ink") == 0);
    bMsCompat &= bResult;
    bAllResult &= bResult;
    outPrintf("str2.Right(3)=>\"%s\" = %s\r\n",
        (LPCSTR)str1, ResultStr(bResult));

    str1 = str2.Right(6);
    bResult = (strcmp(str1, "Strink") == 0);
    bMsCompat &= bResult;
    bAllResult &= bResult;
    outPrintf("str2.Right(6)=>\"%s\" = %s\r\n",
        (LPCSTR)str1, ResultStr(bResult));

    str1 = str2.Right(7);
    bResult = (strcmp(str1, "Strink") == 0);
    bMsCompat &= bResult;
    bAllResult &= bResult;
    outPrintf("str2.Right(7)=>\"%s\" = %s\r\n",
        (LPCSTR)str1, ResultStr(bResult));

    str1 = str2.SpanIncluding("RSTrst");
    bResult = (strcmp(str1, "Str") == 0);
    bMsCompat &= bResult;
    bAllResult &= bResult;
    outPrintf("str2.SpanIncluding(\"RSTrst\")=>\"%s\" = %s\r\n",
        (LPCSTR)str1, ResultStr(bResult));

    str1 = str2.SpanExcluding("IGHigh");
    bResult = (strcmp(str1, "Str") == 0);
    bMsCompat &= bResult;
    bAllResult &= bResult;
    outPrintf("str2.SpanExcluding(\"IGHigh\")=>\"%s\" = %s\r\n",
        (LPCSTR)str1, ResultStr(bResult));

    // Modification methods
    outPrintf("\r\nModification methods\r\n");

    str1 = "Abc123 Def";
    str1.MakeUpper();
    bResult = (strcmp(str1, "ABC123 DEF") == 0);
    bMsCompat &= bResult;
    bAllResult &= bResult;
    outPrintf("str1=\"Abc123 Def\"; str1.MakeUpper()=>\"%s\" = %s\r\n",
        (LPCSTR)str1, ResultStr(bResult));

    str1 = "Abc123 Def";
    str1.MakeLower();
    bResult = (strcmp(str1, "abc123 def") == 0);
    bMsCompat &= bResult;
    bAllResult &= bResult;
    outPrintf("str1=\"Abc123 Def\"; str1.MakeLower()=>\"%s\" = %s\r\n",
        (LPCSTR)str1, ResultStr(bResult));

    str1 = "Abc123 Def";
    str1.MakeReverse();
    bResult = (strcmp(str1, "feD 321cbA") == 0);
    bMsCompat &= bResult;
    bAllResult &= bResult;
    outPrintf("str1=\"Abc123 Def\"; str1.MakeReverse()=>\"%s\" = %s\r\n",
        (LPCSTR)str1, ResultStr(bResult));

    str2.Replace('k', 'g');
    bResult = (strcmp(str2, "String") == 0);
    bMsCompat &= bResult;
    bAllResult &= bResult;
    outPrintf("str2.Replace('k', 'g')=>\"%s\" = %s\r\n",
        (LPCSTR)str2, ResultStr(bResult));

    str1 = "This a very large string.\r\n";
    str1 += "We will test the function 'Replace'\r\n";
    str1 += "with this large string, large, large\r\n";
    str1 += "dfsd fsd fds dsf large dsdlkslkd large  jfdfjds\r\n";

    outPrintf("str1=\"%s\";\r\n", (LPCSTR)str1);

    str3 = "This a very HUGE string.\r\n";
    str3 += "We will test the function 'Replace'\r\n";
    str3 += "with this HUGE string, HUGE, HUGE\r\n";
    str3 += "dfsd fsd fds dsf HUGE dsdlkslkd HUGE  jfdfjds\r\n";

    str1.Replace("large", "HUGE");

    bResult = (strcmp(str1, str3) == 0);
    bMsCompat &= bResult;
    bAllResult &= bResult;
    outPrintf("str1.Replace(\"large\", \"HUGE\")=>\"%s\" = %s\r\n",
        (LPCSTR)str1, ResultStr(bResult));

    str2.Remove('i');
    bResult = (strcmp(str2, "Strng") == 0);
    bMsCompat &= bResult;
    bAllResult &= bResult;
    outPrintf("str2.Remove('i')=>\"%s\" = %s\r\n",
        (LPCSTR)str2, ResultStr(bResult));

    str2.Insert(3, 'i');
    bResult = (strcmp(str2, "String") == 0);
    bMsCompat &= bResult;
    bAllResult &= bResult;
    outPrintf("str2.Insert(3, 'i')=>\"%s\" = %s\r\n",
        (LPCSTR)str2, ResultStr(bResult));

    str2.Insert(6, " Test");
    bResult = (strcmp(str2, "String Test") == 0);
    bMsCompat &= bResult;
    bAllResult &= bResult;
    outPrintf("str2.Insert(6, \" Test\")=>\"%s\" = %s\r\n",
        (LPCSTR)str2, ResultStr(bResult));

    str2.Insert(6, " Quick");
    bResult = (strcmp(str2, "String Quick Test") == 0);
    bMsCompat &= bResult;
    bAllResult &= bResult;
    outPrintf("str2.Insert(6, \" Quick\")=>\"%s\" = %s\r\n",
        (LPCSTR)str2, ResultStr(bResult));

    str2.Delete(6, 6);
    bResult = (strcmp(str2, "String Test") == 0);
    bMsCompat &= bResult;
    bAllResult &= bResult;
    outPrintf("str2.Delete(6, 6)=>\"%s\" = %s\r\n",
        (LPCSTR)str2, ResultStr(bResult));

    str2.Delete(6, 5);
    bResult = (strcmp(str2, "String") == 0);
    bMsCompat &= bResult;
    bAllResult &= bResult;
    outPrintf("str2.Delete(6, 5)=>\"%s\" = %s\r\n",
        (LPCSTR)str2, ResultStr(bResult));

    str1 = "\r\n\t This is a test \t\r\n";
    str1.TrimLeft();
    bResult = (strcmp(str1, "This is a test \t\r\n") == 0);
    bMsCompat &= bResult;
    bAllResult &= bResult;
    outPrintf("str1 = \"\\r\\n\\t This is a test \\t\\r\\n\"\r\n"
	"str1.TrimLeft()=>\"%s\" = %s\r\n",
        (LPCSTR)str1, ResultStr(bResult));

    str1 = "\r\n\t This is a test \t\r\n";
    str1.TrimRight();
    bResult = (strcmp(str1, "\r\n\t This is a test") == 0);
    bMsCompat &= bResult;
    bAllResult &= bResult;
    outPrintf("str1 = \"\\r\\n\\t This is a test \\t\\r\\n\"\r\n"
	"str1.TrimRight()=>\"%s\" = %s\r\n",
        (LPCSTR)str1, ResultStr(bResult));

    str1 = "-*-* Test *-*-";
    str1.TrimLeft(" -*");
    bResult = (strcmp(str1, "Test *-*-") == 0);
    bMsCompat &= bResult;
    bAllResult &= bResult;
    outPrintf("str1 = \"-*-* Test *-*-\";\r\n"
	"str1.TrimLeft(\" -*\")=>\"%s\" = %s\r\n",
        (LPCSTR)str1, ResultStr(bResult));

    str1 = "-*-* Test *-*-";
    str1.TrimRight(" -*");
    bResult = (strcmp(str1, "-*-* Test") == 0);
    bMsCompat &= bResult;
    bAllResult &= bResult;
    outPrintf("str1 = \"-*-* Test *-*-\";\r\n"
	"str1.TrimRight(\" -*\")=>\"%s\" = %s\r\n",
        (LPCSTR)str1, ResultStr(bResult));

    str1 = "--Test--";
    str1.TrimLeft('-');
    bResult = (strcmp(str1, "Test--") == 0);
    bMsCompat &= bResult;
    bAllResult &= bResult;
    outPrintf("str1 = \"--Test--\";\r\n"
	"str1.TrimLeft('-')=>\"%s\" = %s\r\n",
        (LPCSTR)str1, ResultStr(bResult));

    str1 = "--Test--";
    str1.TrimRight('-');
    bResult = (strcmp(str1, "--Test") == 0);
    bMsCompat &= bResult;
    bAllResult &= bResult;
    outPrintf("str1 = \"-- Test --\";\r\n"
	"str1.TrimRight('-')=>\"%s\" = %s\r\n",
        (LPCSTR)str1, ResultStr(bResult));

    // Find methods
    outPrintf("\r\nFind methods\r\n");

    iTmp1 = str2.Find('i');
    bResult = (iTmp1 == 3);
    bMsCompat &= bResult;
    bAllResult &= bResult;
    outPrintf("str2.Find('i')=>%d = %s\r\n",
        iTmp1, ResultStr(bResult));

    iTmp1 = str2.Find("ng");
    bResult = (iTmp1 == 4);
    bMsCompat &= bResult;
    bAllResult &= bResult;
    outPrintf("str2.Find(\"ng\")=>%d = %s\r\n",
        iTmp1, ResultStr(bResult));

    str1 = "Test the test";
    iTmp1 = str1.Find('e', 3);
    bResult = (iTmp1 == 7);
    bMsCompat &= bResult;
    bAllResult &= bResult;
    outPrintf("str1 = \"Test the test\";\r\n"
        "str1.Find('e', 3)=>%d = %s\r\n",
        iTmp1, ResultStr(bResult));

    iTmp1 = str1.Find("est", 3);
    bResult = (iTmp1 == 10);
    bMsCompat &= bResult;
    bAllResult &= bResult;
    outPrintf("str1.Find(\"est\", 3)=>%d = %s\r\n",
        iTmp1, ResultStr(bResult));

    iTmp1 = str1.ReverseFind('h');
    bResult = (iTmp1 == 6);
    bMsCompat &= bResult;
    bAllResult &= bResult;
    outPrintf("str1.ReverseFind('h')=>%d = %s\r\n",
        iTmp1, ResultStr(bResult));

    iTmp1 = str1.FindOneOf("ghi");
    bResult = (iTmp1 == 6);
    bMsCompat &= bResult;
    bAllResult &= bResult;
    outPrintf("str1.FindOneOf(\"ghi\")=>%d = %s\r\n",
        iTmp1, ResultStr(bResult));

    // Buffer methods
    outPrintf("\r\nBuffer methods\r\n");

    LPTSTR pszBuf;
    pszBuf = str1.GetBuffer(100);
    strcpy(pszBuf, "Testing that it allocates as it should!");
    str1.ReleaseBuffer(100);
    iTmp1 = str1.GetLength();
    bResult = (strcmp(str1, "Testing that it allocates as it should!") == 0 &&
        iTmp1 == 100);
    bMsCompat &= bResult;
    bAllResult &= bResult;
    outPrintf("pszBuf = str1.GetBuffer(100);\r\n"
        "strcpy(pszBuf, \"Testing that it allocates as it should!\");\r\n"
        "str1.ReleaseBuffer(100);\r\n"
        "str1=>\"%s\" [len=%d] = %s\r\n",
        (LPCSTR)str1, iTmp1, ResultStr(bResult));

    pszBuf = str1.GetBufferSetLength(50);
    strcpy(pszBuf, "Testing!");
    str1.ReleaseBuffer(50);
    iTmp1 = str1.GetLength();
    bResult = (strcmp(str1, "Testing!") == 0 && iTmp1 == 50);
    bMsCompat &= bResult;
    bAllResult &= bResult;
    outPrintf("pszBuf = str1.GetBufferSetLength(50);\r\n"
        "strcpy(pszBuf, \"Testing!\");\r\n"
        "str1.ReleaseBuffer(50);\r\n"
        "str1=>\"%s\" [len=%d] = %s\r\n",
        (LPCSTR)str1, iTmp1, ResultStr(bResult));

    pszBuf = str1.LockBuffer();
    strcpy(pszBuf, "Test (Un)LockBuffer!");
    str1.ReleaseBuffer(50);
    iTmp1 = str1.GetLength();
    bResult = (strcmp(str1, "Test (Un)LockBuffer!") == 0 && iTmp1 == 50);
    bMsCompat &= bResult;
    bAllResult &= bResult;
    outPrintf("pszBuf = str1.LockBuffer();\r\n"
        "strcpy(pszBuf, \"Test (Un)LockBuffer!\");\r\n"
        "str1.UnlockBuffer();\r\n"
        "str1=>\"%s\" [len=%d] = %s\r\n",
        (LPCSTR)str1, iTmp1, ResultStr(bResult));

    str1.GetBuffer(0);
    str1.ReleaseBuffer();
    iTmp1 = str1.GetLength();
    bResult = (iTmp1 == (int)strlen(str1));
    bMsCompat &= bResult;
    bAllResult &= bResult;
    outPrintf("str1.GetBuffer(0);\r\n"
        "str1.ReleaseBuffer();\r\n"
        "str1.GetLength()=>%d = %s\r\n",
        iTmp1, ResultStr(bResult));

    // !! There is no way to test that FreeExtra() works correctly,
    // !! because we cannot access the internal value nAllocLength

    // SysString methods
    outPrintf("\r\nWindows specific methods\r\n");

    char szBuf[40];
    BSTR bstr;
    str1 = "Testing";
    bstr = str1.AllocSysString();
    wcstombs(szBuf, bstr, sizeof(szBuf));
    bResult = (strcmp(szBuf, str1) == 0);
    bMsCompat &= bResult;
    bAllResult &= bResult;
    outPrintf("str1 = \"Testing\"; bstr = str1.AllocSysString();\r\n"
        "bstr=>\"%s\" = %s\r\n",
        (LPCSTR)szBuf, ResultStr(bResult));

    str2 = "Test #2";
    str2.SetSysString(&bstr);
    wcstombs(szBuf, bstr, sizeof(szBuf));
    bResult = (strcmp(szBuf, str2) == 0);
    bMsCompat &= bResult;
    bAllResult &= bResult;
    outPrintf("str2 = \"Test #2\"; str2.SetSysString(&bstr);\r\n"
        "bstr=>\"%s\" = %s\r\n",
        (LPCSTR)szBuf, ResultStr(bResult));

    ::SysFreeString(bstr);

    str1.LoadString(IDS_TESTSTRING);
    bResult = (strcmp(str1, (char*)g_szTestString) == 0);
    bMsCompat &= bResult;
    bAllResult &= bResult;
    outPrintf("str1.LoadString(IDS_TESTSTRING);\r\n"
        "str1=>\"%s\" = %s\r\n",
        (LPCSTR)str1, ResultStr(bResult));
    if ( !bResult ) {
        int ii;
        for (ii=0 ; ii<(int)strlen(str1) ; ii++) {
            if (str1[ii] != (char)g_szTestString[ii]) {
                outPrintf("str1[%d] = '%c' != g_szTestString[%d] = '%c'\r\n",
                    ii, str1[ii], ii, (char)g_szTestString[ii]);
            }
        }
    }

    str1.AnsiToOem();
    bResult = (strcmp(str1, (char*)g_szTestStringOem) == 0);
    bMsCompat &= bResult;
    bAllResult &= bResult;
    outPrintf("str1.AnsiToOem();\r\n"
        "str1=>\"%s\" = %s\r\n",
        (LPCSTR)str1, ResultStr(bResult));
    if ( !bResult ) {
        int ii;
        for (ii=0 ; ii<(int)strlen(str1) ; ii++) {
            if (str1[ii] != (char)g_szTestString[ii]) {
                outPrintf("str1[%d] = '%c' != g_szTestString[%d] = '%c'\r\n",
                    ii, str1[ii], ii, (char)g_szTestString[ii]);
            }
        }
    }

    str1.OemToAnsi();
    bResult = (strcmp(str1, (char*)g_szTestString) == 0);
    bMsCompat &= bResult;
    bAllResult &= bResult;
    outPrintf("str1.OemToAnsi();\r\n"
        "str1=>\"%s\" = %s\r\n",
        (LPCSTR)str1, ResultStr(bResult));

    // Comparison operators
    outPrintf("\r\nComparison operators\r\n");

    str1 = "Abc";
    str2 = "Abc";
    bTmp1 = (str1 == str2);
    bTmp2 = (str1 != str2);
    bTmp3 = (str1 > str2);
    bTmp4 = (str1 >= str2);
    bTmp5 = (str1 < str2);
    bTmp6 = (str1 <= str2);
    bResult = ((bTmp1 == TRUE) && (bTmp2 == FALSE) && (bTmp3 == FALSE) &&
        (bTmp4 == TRUE) && (bTmp5 == FALSE) && (bTmp6 == TRUE));
    outPrintf("str1 = \"Abc\"; str2 = \"Abc\"; str1 op str2:\r\n"
        "==:%d, !=:%d, >:%d, >=:%d, <:%d, <=:%d = %s\r\n",
        bTmp1, bTmp2, bTmp3, bTmp4, bTmp5, bTmp6, ResultStr(bResult));

    str1 = "ABC";
    str2 = "abc";
    bTmp1 = (str1 == str2);
    bTmp2 = (str1 != str2);
    bTmp3 = (str1 > str2);
    bTmp4 = (str1 >= str2);
    bTmp5 = (str1 < str2);
    bTmp6 = (str1 <= str2);
    bResult = ((bTmp1 == FALSE) && (bTmp2 == TRUE) && (bTmp3 == FALSE) &&
        (bTmp4 == FALSE) && (bTmp5 == TRUE) && (bTmp6 == TRUE));
    outPrintf("str1 = \"ABC\"; str2 = \"abc\"; str1 op str2:\r\n"
        "==:%d, !=:%d, >:%d, >=:%d, <:%d, <=:%d = %s\r\n",
        bTmp1, bTmp2, bTmp3, bTmp4, bTmp5, bTmp6, ResultStr(bResult));

    str1 = "Abc";
    str2 = "Def";
    bTmp1 = (str1 == str2);
    bTmp2 = (str1 != str2);
    bTmp3 = (str1 > str2);
    bTmp4 = (str1 >= str2);
    bTmp5 = (str1 < str2);
    bTmp6 = (str1 <= str2);
    bResult = ((bTmp1 == FALSE) && (bTmp2 == TRUE) && (bTmp3 == FALSE) &&
        (bTmp4 == FALSE) && (bTmp5 == TRUE) && (bTmp6 == TRUE));
    outPrintf("str1 = \"Abc\"; str2 = \"Def\"; str1 op str2:\r\n"
        "==:%d, !=:%d, >:%d, >=:%d, <:%d, <=:%d = %s\r\n",
        bTmp1, bTmp2, bTmp3, bTmp4, bTmp5, bTmp6, ResultStr(bResult));

    str1 = "Def";
    str2 = "Abc";
    bTmp1 = (str1 == str2);
    bTmp2 = (str1 != str2);
    bTmp3 = (str1 > str2);
    bTmp4 = (str1 >= str2);
    bTmp5 = (str1 < str2);
    bTmp6 = (str1 <= str2);
    bResult = ((bTmp1 == FALSE) && (bTmp2 == TRUE) && (bTmp3 == TRUE) &&
        (bTmp4 == TRUE) && (bTmp5 == FALSE) && (bTmp6 == FALSE));
    outPrintf("str1 = \"Def\"; str2 = \"Abc\"; str1 op str2:\r\n"
        "==:%d, !=:%d, >:%d, >=:%d, <:%d, <=:%d = %s\r\n",
        bTmp1, bTmp2, bTmp3, bTmp4, bTmp5, bTmp6, ResultStr(bResult));

    str1 = "Abc";
    strcpy(szBuf, "Abc");
    bTmp1 = (str1 == szBuf);
    bTmp2 = (str1 != szBuf);
    bTmp3 = (str1 > szBuf);
    bTmp4 = (str1 >= szBuf);
    bTmp5 = (str1 < szBuf);
    bTmp6 = (str1 <= szBuf);
    bResult = ((bTmp1 == TRUE) && (bTmp2 == FALSE) && (bTmp3 == FALSE) &&
        (bTmp4 == TRUE) && (bTmp5 == FALSE) && (bTmp6 == TRUE));
    outPrintf("str1 = \"Abc\"; strcpy(szBuf, \"Abc\"); str1 op szBuf:\r\n"
        "==:%d, !=:%d, >:%d, >=:%d, <:%d, <=:%d = %s\r\n",
        bTmp1, bTmp2, bTmp3, bTmp4, bTmp5, bTmp6, ResultStr(bResult));

    str1 = "ABC";
    strcpy(szBuf, "abc");
    bTmp1 = (str1 == szBuf);
    bTmp2 = (str1 != szBuf);
    bTmp3 = (str1 > szBuf);
    bTmp4 = (str1 >= szBuf);
    bTmp5 = (str1 < szBuf);
    bTmp6 = (str1 <= szBuf);
    bResult = ((bTmp1 == FALSE) && (bTmp2 == TRUE) && (bTmp3 == FALSE) &&
        (bTmp4 == FALSE) && (bTmp5 == TRUE) && (bTmp6 == TRUE));
    outPrintf("str1 = \"ABC\"; strcpy(szBuf, \"abc\"); str1 op szBuf:\r\n"
        "==:%d, !=:%d, >:%d, >=:%d, <:%d, <=:%d = %s\r\n",
        bTmp1, bTmp2, bTmp3, bTmp4, bTmp5, bTmp6, ResultStr(bResult));

    str1 = "Abc";
    strcpy(szBuf, "Def");
    bTmp1 = (str1 == szBuf);
    bTmp2 = (str1 != szBuf);
    bTmp3 = (str1 > szBuf);
    bTmp4 = (str1 >= szBuf);
    bTmp5 = (str1 < szBuf);
    bTmp6 = (str1 <= szBuf);
    bResult = ((bTmp1 == FALSE) && (bTmp2 == TRUE) && (bTmp3 == FALSE) &&
        (bTmp4 == FALSE) && (bTmp5 == TRUE) && (bTmp6 == TRUE));
    outPrintf("str1 = \"Abc\"; strcpy(szBuf, \"Def\"); str1 op szBuf:\r\n"
        "==:%d, !=:%d, >:%d, >=:%d, <:%d, <=:%d = %s\r\n",
        bTmp1, bTmp2, bTmp3, bTmp4, bTmp5, bTmp6, ResultStr(bResult));

    str1 = "Def";
    strcpy(szBuf, "Abc");
    bTmp1 = (str1 == szBuf);
    bTmp2 = (str1 != szBuf);
    bTmp3 = (str1 > szBuf);
    bTmp4 = (str1 >= szBuf);
    bTmp5 = (str1 < szBuf);
    bTmp6 = (str1 <= szBuf);
    bResult = ((bTmp1 == FALSE) && (bTmp2 == TRUE) && (bTmp3 == TRUE) &&
        (bTmp4 == TRUE) && (bTmp5 == FALSE) && (bTmp6 == FALSE));
    outPrintf("str1 = \"Def\"; strcpy(szBuf, \"Abc\"); str1 op szBuf:\r\n"
        "==:%d, !=:%d, >:%d, >=:%d, <:%d, <=:%d = %s\r\n",
        bTmp1, bTmp2, bTmp3, bTmp4, bTmp5, bTmp6, ResultStr(bResult));

    strcpy(szBuf, "Abc");
    str2 = "Abc";
    bTmp1 = (szBuf == str2);
    bTmp2 = (szBuf != str2);
    bTmp3 = (szBuf > str2);
    bTmp4 = (szBuf >= str2);
    bTmp5 = (szBuf < str2);
    bTmp6 = (szBuf <= str2);
    bResult = ((bTmp1 == TRUE) && (bTmp2 == FALSE) && (bTmp3 == FALSE) &&
        (bTmp4 == TRUE) && (bTmp5 == FALSE) && (bTmp6 == TRUE));
    outPrintf("strcpy(szBuf, \"Abc\"); str2 = \"Abc\"; szBuf op str2:\r\n"
        "==:%d, !=:%d, >:%d, >=:%d, <:%d, <=:%d = %s\r\n",
        bTmp1, bTmp2, bTmp3, bTmp4, bTmp5, bTmp6, ResultStr(bResult));

    strcpy(szBuf, "ABC");
    str2 = "abc";
    bTmp1 = (szBuf == str2);
    bTmp2 = (szBuf != str2);
    bTmp3 = (szBuf > str2);
    bTmp4 = (szBuf >= str2);
    bTmp5 = (szBuf < str2);
    bTmp6 = (szBuf <= str2);
    bResult = ((bTmp1 == FALSE) && (bTmp2 == TRUE) && (bTmp3 == FALSE) &&
        (bTmp4 == FALSE) && (bTmp5 == TRUE) && (bTmp6 == TRUE));
    outPrintf("strcpy(szBuf, \"ABC\"); str2 = \"abc\"; szBuf op str2:\r\n"
        "==:%d, !=:%d, >:%d, >=:%d, <:%d, <=:%d = %s\r\n",
        bTmp1, bTmp2, bTmp3, bTmp4, bTmp5, bTmp6, ResultStr(bResult));

    strcpy(szBuf, "Abc");
    str2 = "Def";
    bTmp1 = (szBuf == str2);
    bTmp2 = (szBuf != str2);
    bTmp3 = (szBuf > str2);
    bTmp4 = (szBuf >= str2);
    bTmp5 = (szBuf < str2);
    bTmp6 = (szBuf <= str2);
    bResult = ((bTmp1 == FALSE) && (bTmp2 == TRUE) && (bTmp3 == FALSE) &&
        (bTmp4 == FALSE) && (bTmp5 == TRUE) && (bTmp6 == TRUE));
    outPrintf("strcpy(szBuf, \"Abc\"); str2 = \"Def\"; szBuf op str2:\r\n"
        "==:%d, !=:%d, >:%d, >=:%d, <:%d, <=:%d = %s\r\n",
        bTmp1, bTmp2, bTmp3, bTmp4, bTmp5, bTmp6, ResultStr(bResult));

    strcpy(szBuf, "Def");
    str2 = "Abc";
    bTmp1 = (szBuf == str2);
    bTmp2 = (szBuf != str2);
    bTmp3 = (szBuf > str2);
    bTmp4 = (szBuf >= str2);
    bTmp5 = (szBuf < str2);
    bTmp6 = (szBuf <= str2);
    bResult = ((bTmp1 == FALSE) && (bTmp2 == TRUE) && (bTmp3 == TRUE) &&
        (bTmp4 == TRUE) && (bTmp5 == FALSE) && (bTmp6 == FALSE));
    outPrintf("strcpy(szBuf, \"Def\"); str2 = \"Abc\"; szBuf op str2:\r\n"
        "==:%d, !=:%d, >:%d, >=:%d, <:%d, <=:%d = %s\r\n",
        bTmp1, bTmp2, bTmp3, bTmp4, bTmp5, bTmp6, ResultStr(bResult));

    /*
      == CString and LPCSTR either side
      !=
      >
      >=
      <
      <=
     */

    outPrintf("\r\n"
        "Summary\r\n"
        "-------\r\n"
        "MS Compatibility   = %s\r\n"
        "Full Functionality = %s\r\n"
        "NOTE: !!! No Unicode compatibility is supported at this time !!!",
        ResultStr(bMsCompat),
        ResultStr(bAllResult));
}
