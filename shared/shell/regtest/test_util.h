/*!
 *
 * @file test_util.h
 *
 * @brief Minimal C89 test harness for Win16 SHELL.DLL tests.
 *
 * Win16 has no console subsystem: printf() does not reach the user.
 * All test output is written to a log file, opened by test_init().
 *
 * (c) osFree Project 2026, <http://www.osFree.org>
 * for licence see licence.txt in root directory, or project website
 */

#ifndef CCL_TEST_UTIL_H
#define CCL_TEST_UTIL_H

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * @brief Open the log file. Call once before any TEST_RUN.
 * @param pszLogPath Path of the log file (e.g. "TEST.LOG").
 */
void test_init(const char *pszLogPath);

/*!
 * @brief Print the name of the test about to run.
 */
void test_start(const char *pszName);

/*!
 * @brief Record a successful assertion.
 */
void test_pass(void);

/*!
 * @brief Record a failed assertion and print its location.
 */
void test_fail(const char *pszFile, int iLine, const char *pszExpr);

/*!
 * @brief Close the log file and print the summary.
 *
 * @return 0 if all assertions passed, 1 otherwise.
 */
int test_summary(void);

#define TEST_RUN(fn)                                                     \
    do {                                                                 \
        test_start(#fn);                                                 \
        fn();                                                            \
    } while (0)

#define TEST_ASSERT(cond)                                                \
    do {                                                                 \
        if (cond) {                                                      \
            test_pass();                                                 \
        } else {                                                         \
            test_fail(__FILE__, __LINE__, #cond);                        \
            return;                                                      \
        }                                                                \
    } while (0)

#define TEST_ASSERT_MSG(cond, msg)                                       \
    do {                                                                 \
        if (cond) {                                                      \
            test_pass();                                                 \
        } else {                                                         \
            test_fail(__FILE__, __LINE__, (msg));                        \
            return;                                                      \
        }                                                                \
    } while (0)

#ifdef __cplusplus
}
#endif

#endif /* CCL_TEST_UTIL_H */
