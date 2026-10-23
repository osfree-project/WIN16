/*!
 *
 * @file test_util.c
 *
 * @brief Implementation of the C89 test harness for Win16 SHELL.DLL.
 *
 * Win16 has no console subsystem, so every message goes into a log file
 * opened by test_init(). The log is flushed after every line so that a
 * crash leaves behind everything up to the last successful assertion.
 *
 * (c) osFree Project 2026, <http://www.osFree.org>
 * for licence see licence.txt in root directory, or project website
 */

#include <stdio.h>
#include <stdarg.h>
#include "test_util.h"

/*! @brief Number of successful assertions so far. */
static int   g_iPass = 0;

/*! @brief Number of failed assertions so far. */
static int   g_iFail = 0;

/*! @brief Log file, or NULL if not yet opened or already closed. */
static FILE *g_pLog  = NULL;

/*!
 * @brief Open the log file. Call once before any TEST_RUN.
 *
 * @param[in] pszLogPath Path of the log file (e.g. "TEST.LOG"), or NULL
 *                       to use the default "TEST.LOG".
 */
void test_init(const char *pszLogPath)
{
    g_pLog = fopen(pszLogPath ? pszLogPath : "TEST.LOG", "w");
    if (g_pLog) {
        fprintf(g_pLog, "osFree SHELL.DLL registry test\n");
        fflush(g_pLog);
    }
}

/*!
 * @brief Write one formatted line into the log (internal helper).
 *
 * @param[in] pszFmt printf-style format string.
 * @param[in] ...    Format arguments.
 */
static void test_printf(const char *pszFmt, ...)
{
    va_list ap;

    if (!g_pLog) return;

    va_start(ap, pszFmt);
    vfprintf(g_pLog, pszFmt, ap);
    va_end(ap);
    fflush(g_pLog);
}

/*!
 * @brief Print the name of the test about to run.
 *
 * @param[in] pszName Name of the test (usually the function name).
 */
void test_start(const char *pszName)
{
    test_printf("== %s\n", pszName ? pszName : "(unnamed)");
}

/*!
 * @brief Record a successful assertion.
 */
void test_pass(void)
{
    g_iPass++;
}

/*!
 * @brief Record a failed assertion and print its location.
 *
 * @param[in] pszFile Source file name.
 * @param[in] iLine   Source line number.
 * @param[in] pszExpr Failed expression text.
 */
void test_fail(const char *pszFile, int iLine, const char *pszExpr)
{
    g_iFail++;
    test_printf("   FAIL %s:%d: %s\n",
                pszFile ? pszFile : "?",
                iLine,
                pszExpr ? pszExpr : "?");
}

/*!
 * @brief Print the summary line and close the log file.
 *
 * @return 0 if all assertions passed, 1 otherwise.
 *
 * @retval 0 All assertions passed.
 * @retval 1 At least one assertion failed.
 */
int test_summary(void)
{
    test_printf("---\n%d passed, %d failed\n", g_iPass, g_iFail);
    if (g_pLog) {
        fclose(g_pLog);
        g_pLog = NULL;
    }
    return (g_iFail == 0) ? 0 : 1;
}
