/*!
 *
 * @file regtest.c
 *
 * @brief Unit tests for the Win16 SHELL.DLL registry implementation.
 *
 * Built as a Win16 GUI application (WinMain), linked against shell.lib.
 * Output goes to TEST.LOG; the final PASS/FAIL summary is shown in a
 * MessageBox. Run under a real Windows 3.1, otvdm, or DOSBox-X with a
 * SHELL.DLL that exports the RegXxx functions.
 *
 * (c) osFree Project 2026, <http://www.osFree.org>
 * for licence see licence.txt in root directory, or project website
 */

#include <stdio.h>
#include <string.h>
#include <windows.h>
#include <shellapi.h>

#include "test_util.h"

/* Scratch namespace shared by all tests. Created under HKEY_CLASSES_ROOT
   and deleted at the end of each test, so leftover state from a crashed
   run cannot leak into the next one. */
/*! @def TEST_KEY
    @brief Scratch key name used by all create/set/delete tests. */
#define TEST_KEY "osFreeSHELLTestKey"

/* ------------------------------------------------------------------ */
/* HKEY_CLASSES_ROOT as 0 and as 1                                    */
/* ------------------------------------------------------------------ */

/*! @brief RegOpenKey accepts 0 as HKEY_CLASSES_ROOT. */
static void test_root_as_0(void)
{
    HKEY hk;
    LONG ret;

    ret = RegOpenKey(0, ".txt", &hk);
    TEST_ASSERT(ret == ERROR_SUCCESS);
    TEST_ASSERT(hk != 0);

    RegCloseKey(hk);
}

/*! @brief RegOpenKey accepts 1 as HKEY_CLASSES_ROOT. */
static void test_root_as_1(void)
{
    HKEY hk;
    LONG ret;

    ret = RegOpenKey(1, ".txt", &hk);
    TEST_ASSERT(ret == ERROR_SUCCESS);
    TEST_ASSERT(hk != 0);

    RegCloseKey(hk);
}

/*! @brief RegOpenKey returns ERROR_BADKEY for a missing subkey. */
static void test_root_nonexistent(void)
{
    HKEY hk;
    LONG ret;

    ret = RegOpenKey(0, "ThisKeyDoesNotExist", &hk);
    TEST_ASSERT(ret == ERROR_BADKEY);
    TEST_ASSERT(hk == 0);
}

/* ------------------------------------------------------------------ */
/* Stock REG.DAT values                                               */
/* ------------------------------------------------------------------ */

/*! @brief HKEY_CLASSES_ROOT\\.txt defaults to "txtfile". */
static void test_txt_default_value(void)
{
    char buf[256];
    LONG cb, ret;

    cb = (LONG)sizeof(buf);
    ret = RegQueryValue(0, ".txt", buf, &cb);
    TEST_ASSERT(ret == ERROR_SUCCESS);
    TEST_ASSERT(strcmp(buf, "txtfile") == 0);
    TEST_ASSERT(cb == (LONG)strlen("txtfile") + 1);
}

/*! @brief HKEY_CLASSES_ROOT\\.ini defaults to "txtfile". */
static void test_ini_default_value(void)
{
    char buf[256];
    LONG cb, ret;

    cb = (LONG)sizeof(buf);
    ret = RegQueryValue(0, ".ini", buf, &cb);
    TEST_ASSERT(ret == ERROR_SUCCESS);
    TEST_ASSERT(strcmp(buf, "txtfile") == 0);
}

/*! @brief HKEY_CLASSES_ROOT\\txtfile\\shell\\open\\command is notepad.exe %1. */
static void test_command_for_txt(void)
{
    char buf[256];
    LONG cb, ret;

    cb = (LONG)sizeof(buf);
    ret = RegQueryValue(0, "txtfile\\shell\\open\\command", buf, &cb);
    TEST_ASSERT(ret == ERROR_SUCCESS);
    TEST_ASSERT(strcmp(buf, "notepad.exe %1") == 0);
}

/*! @brief HKEY_CLASSES_ROOT\\PBrush\\shell\\open\\command is pbrush.exe %1. */
static void test_command_for_bmp(void)
{
    char buf[256];
    LONG cb, ret;

    cb = (LONG)sizeof(buf);
    ret = RegQueryValue(0, "PBrush\\shell\\open\\command", buf, &cb);
    TEST_ASSERT(ret == ERROR_SUCCESS);
    TEST_ASSERT(strcmp(buf, "pbrush.exe %1") == 0);
}

/* ------------------------------------------------------------------ */
/* Create / set / query / delete round trip                           */
/* ------------------------------------------------------------------ */

/*! @brief Full create / set / query / delete round trip. */
static void test_create_set_query_delete(void)
{
    HKEY hk;
    char buf[256];
    LONG cb, ret;

    RegDeleteKey(0, TEST_KEY);

    ret = RegCreateKey(0, TEST_KEY, &hk);
    TEST_ASSERT(ret == ERROR_SUCCESS);
    TEST_ASSERT(hk != 0);

    ret = RegSetValue(hk, NULL, REG_SZ, "hello", 5);
    TEST_ASSERT(ret == ERROR_SUCCESS);

    cb = (LONG)sizeof(buf);
    ret = RegQueryValue(hk, NULL, buf, &cb);
    TEST_ASSERT(ret == ERROR_SUCCESS);
    TEST_ASSERT(strcmp(buf, "hello") == 0);
    TEST_ASSERT(cb == 6);

    RegCloseKey(hk);

    cb = (LONG)sizeof(buf);
    ret = RegQueryValue(0, TEST_KEY, buf, &cb);
    TEST_ASSERT(ret == ERROR_SUCCESS);
    TEST_ASSERT(strcmp(buf, "hello") == 0);

    ret = RegSetValue(0, TEST_KEY, REG_SZ, "world", 5);
    TEST_ASSERT(ret == ERROR_SUCCESS);

    cb = (LONG)sizeof(buf);
    ret = RegQueryValue(0, TEST_KEY, buf, &cb);
    TEST_ASSERT(ret == ERROR_SUCCESS);
    TEST_ASSERT(strcmp(buf, "world") == 0);

    ret = RegDeleteKey(0, TEST_KEY);
    TEST_ASSERT(ret == ERROR_SUCCESS);

    ret = RegOpenKey(0, TEST_KEY, &hk);
    TEST_ASSERT(ret == ERROR_BADKEY);
}

/*! @brief RegCreateKey on an existing key returns ERROR_SUCCESS. */
static void test_create_existing(void)
{
    HKEY hk1, hk2;
    LONG ret;

    RegDeleteKey(0, TEST_KEY);

    ret = RegCreateKey(0, TEST_KEY, &hk1);
    TEST_ASSERT(ret == ERROR_SUCCESS);
    RegCloseKey(hk1);

    ret = RegCreateKey(0, TEST_KEY, &hk2);
    TEST_ASSERT(ret == ERROR_SUCCESS);
    TEST_ASSERT(hk2 != 0);

    RegCloseKey(hk2);
    RegDeleteKey(0, TEST_KEY);
}

/* ------------------------------------------------------------------ */
/* Small buffer returns ERROR_BADKEY (Win16 has no ERROR_MORE_DATA)   */
/* ------------------------------------------------------------------ */

/*! @brief A too-small buffer returns ERROR_BADKEY with updated cb. */
static void test_small_buffer(void)
{
    HKEY hk;
    char buf[4];
    LONG cb, ret;

    RegDeleteKey(0, TEST_KEY);

    ret = RegCreateKey(0, TEST_KEY, &hk);
    TEST_ASSERT(ret == ERROR_SUCCESS);

    ret = RegSetValue(hk, NULL, REG_SZ, "abcdefghij", 10);
    TEST_ASSERT(ret == ERROR_SUCCESS);

    cb = (LONG)sizeof(buf);
    ret = RegQueryValue(hk, NULL, buf, &cb);
    TEST_ASSERT(ret == ERROR_BADKEY);
    TEST_ASSERT(cb == 11);   /* updated to required size */

    RegCloseKey(hk);
    RegDeleteKey(0, TEST_KEY);
}

/* ------------------------------------------------------------------ */
/* RegEnumKey                                                         */
/* ------------------------------------------------------------------ */

/*! @brief Enumerate subkeys in creation order. */
static void test_enum_keys(void)
{
    HKEY hk, sub;
    char buf[256];
    LONG ret;

    RegDeleteKey(0, TEST_KEY);

    ret = RegCreateKey(0, TEST_KEY, &hk);
    TEST_ASSERT(ret == ERROR_SUCCESS);

    ret = RegCreateKey(hk, "Sub1", &sub);
    TEST_ASSERT(ret == ERROR_SUCCESS);
    RegCloseKey(sub);

    ret = RegCreateKey(hk, "Sub2", &sub);
    TEST_ASSERT(ret == ERROR_SUCCESS);
    RegCloseKey(sub);

    ret = RegCreateKey(hk, "Sub3", &sub);
    TEST_ASSERT(ret == ERROR_SUCCESS);
    RegCloseKey(sub);

    ret = RegEnumKey(hk, 0, buf, sizeof(buf));
    TEST_ASSERT(ret == ERROR_SUCCESS);
    TEST_ASSERT(strcmp(buf, "Sub1") == 0);

    ret = RegEnumKey(hk, 1, buf, sizeof(buf));
    TEST_ASSERT(ret == ERROR_SUCCESS);
    TEST_ASSERT(strcmp(buf, "Sub2") == 0);

    ret = RegEnumKey(hk, 2, buf, sizeof(buf));
    TEST_ASSERT(ret == ERROR_SUCCESS);
    TEST_ASSERT(strcmp(buf, "Sub3") == 0);

    /* Past the end -> ERROR_BADKEY (Win16 has no ERROR_NO_MORE_ITEMS). */
    ret = RegEnumKey(hk, 3, buf, sizeof(buf));
    TEST_ASSERT(ret == ERROR_BADKEY);

    RegCloseKey(hk);
    RegDeleteKey(0, TEST_KEY);
}

/* ------------------------------------------------------------------ */
/* RegCloseKey                                                        */
/* ------------------------------------------------------------------ */

/*! @brief RegCloseKey accepts subkeys and both root spellings. */
static void test_close_key(void)
{
    HKEY hk;
    LONG ret;

    ret = RegOpenKey(0, ".txt", &hk);
    TEST_ASSERT(ret == ERROR_SUCCESS);

    ret = RegCloseKey(hk);
    TEST_ASSERT(ret == ERROR_SUCCESS);

    /* Closing root as 0 and as 1 must both succeed. */
    ret = RegCloseKey(0);
    TEST_ASSERT(ret == ERROR_SUCCESS);

    ret = RegCloseKey(1);
    TEST_ASSERT(ret == ERROR_SUCCESS);
}

/* ------------------------------------------------------------------ */
/* Root cannot be deleted                                             */
/* ------------------------------------------------------------------ */

/*! @brief RegDeleteKey on the root key returns ERROR_BADKEY. */
static void test_delete_root(void)
{
    LONG ret;

    ret = RegDeleteKey(0, "");
    TEST_ASSERT(ret == ERROR_BADKEY);

    ret = RegDeleteKey(1, "");
    TEST_ASSERT(ret == ERROR_BADKEY);
}

/* ------------------------------------------------------------------ */
/* NULL output pointer                                                */
/* ------------------------------------------------------------------ */

/*! @brief RegOpenKey with NULL phkResult returns ERROR_BADKEY. */
static void test_null_phkresult(void)
{
    LONG ret;

    ret = RegOpenKey(0, ".txt", NULL);
    TEST_ASSERT(ret == ERROR_BADKEY);
}

/* ------------------------------------------------------------------ */
/* Entry point                                                        */
/* ------------------------------------------------------------------ */

/*! @brief WinMain: run all tests, write TEST.LOG, show PASS/FAIL box.

    @param[in] hInstance     Current instance handle (unused).
    @param[in] hPrevInstance Previous instance handle (unused).
    @param[in] lpCmdLine     Command line (unused).
    @param[in] nCmdShow      Show command (unused).

    @return 0 if all tests passed, non-zero otherwise.

    @retval 0 All tests passed.
    @retval 1 At least one test failed.
*/
int PASCAL WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine, int nCmdShow)
{
    int rc;
    char szSummary[256];

    (void)hInstance;
    (void)hPrevInstance;
    (void)lpCmdLine;
    (void)nCmdShow;

    test_init("TEST.LOG");

    TEST_RUN(test_root_as_0);
    TEST_RUN(test_root_as_1);
    TEST_RUN(test_root_nonexistent);
    TEST_RUN(test_txt_default_value);
    TEST_RUN(test_ini_default_value);
    TEST_RUN(test_command_for_txt);
    TEST_RUN(test_command_for_bmp);
    TEST_RUN(test_create_set_query_delete);
    TEST_RUN(test_create_existing);
    TEST_RUN(test_small_buffer);
    TEST_RUN(test_enum_keys);
    TEST_RUN(test_close_key);
    TEST_RUN(test_delete_root);
    TEST_RUN(test_null_phkresult);

    rc = test_summary();

    sprintf(szSummary, "osFree SHELL.DLL registry test\n%s\n\n"
                       "See TEST.LOG for details.",
            rc == 0 ? "PASS" : "FAIL");
    MessageBox((HWND)0, szSummary, "Registry tests",
               MB_OK | (rc == 0 ? MB_ICONINFORMATION : MB_ICONHAND));

    return rc;
}
