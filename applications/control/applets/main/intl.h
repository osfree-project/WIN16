#ifndef INTL_H
#define INTL_H

#include <windows.h>
#include <dos.h>
#include <string.h>
#include <stdlib.h>
#include "main.h"
#include "winnls.h"

/* ---------- Resource IDs ---------- */
#define IDI_INTL                    100

#define DLG_DATEFMT                 210   /* изменено для избежания конфликта с currency.rc */
#define DLG_TIMEFMT                 211

/* ID для основного диалога */
#define IDC_INTL_COUNTRY            300
#define IDC_INTL_LANGUAGE           301
#define IDC_INTL_KEYBOARD           302
#define IDC_INTL_MEASURE            303
#define IDC_INTL_LISTSEP            304
#define IDC_INTL_DATE_SHORT         305
#define IDC_INTL_DATE_LONG          306
#define IDC_INTL_TIME_SAMPLE        307
#define IDC_INTL_CURR_POS           308
#define IDC_INTL_CURR_NEG           309
#define IDC_INTL_NUM_SAMPLE         310
#define IDC_INTL_DATE_CHANGE        311
#define IDC_INTL_TIME_CHANGE        312
#define IDC_INTL_CURR_CHANGE        313
#define IDC_INTL_NUM_CHANGE         314

/* Акселераторы */
#define IDC_INTL_ACCEL_DATE         400
#define IDC_INTL_ACCEL_TIME         401
#define IDC_INTL_ACCEL_CURR         402
#define IDC_INTL_ACCEL_NUM          403

/* Групповые рамки основного диалога */
#define IDC_INTL_DATE_GROUP         370
#define IDC_INTL_TIME_GROUP         371
#define IDC_INTL_CURR_GROUP         372
#define IDC_INTL_NUM_GROUP          373

/* ID для диалога Date Format */
#define IDC_DATEFMT_S_MDY           320
#define IDC_DATEFMT_S_DMY           321
#define IDC_DATEFMT_S_YMD           322
#define IDC_DATEFMT_SEPARATOR       323
#define IDC_DATEFMT_S_DAYLZ         324
#define IDC_DATEFMT_S_MONTHLZ       325
#define IDC_DATEFMT_S_CENTURY       326
#define IDC_DATEFMT_L_MDY           327
#define IDC_DATEFMT_L_DMY           328
#define IDC_DATEFMT_L_YMD           329
#define IDC_DATEFMT_L_COMBO1        330
#define IDC_DATEFMT_L_EDIT1         331
#define IDC_DATEFMT_L_COMBO2        332
#define IDC_DATEFMT_L_EDIT2         333
#define IDC_DATEFMT_L_COMBO3        334
#define IDC_DATEFMT_L_EDIT3         335
#define IDC_DATEFMT_L_COMBO4        336
#define IDC_DATEFMT_L_SAMPLE        337

/* ID для диалога Time Format */
#define IDC_TIMEFMT_12H             340
#define IDC_TIMEFMT_24H             341
#define IDC_TIMEFMT_AM              342
#define IDC_TIMEFMT_PM              343
#define IDC_TIMEFMT_SEP             344
#define IDC_TIMEFMT_LZ_OFF          345
#define IDC_TIMEFMT_LZ_ON           346

#include "intl_number.h"
#include "intl_currency.h"

/* ---------- общие глобальные переменные ---------- */
extern char g_iniCountry[8];
extern char g_iniLanguage[32];
extern char g_iniKeyboard[32];
extern int  g_iniMeasure;
extern char g_iniListSep[4];
extern int  g_iniDateFormat;
extern int  g_iniTimeFormat;
extern int  g_iniCurrencyFmt;
extern char g_iniCurrencySym[8];
extern char g_iniDecimal[4];
extern char g_iniThousand[4];
extern int  g_iniDigits;
extern int  g_iniLZero;
extern char g_szTimeSep[4];
extern char g_szAm[8];
extern char g_szPm[8];
extern int  g_iniTLZero;
extern char g_szDateSep[4];
extern char g_szLongDateFmt[80];
extern char g_szShortDateFmt[80];
extern int  g_iniNegCurr;
extern UINT  g_iniGrouping;
extern UINT  g_iniNegNumber;

/* ---------- прототипы общих функций ---------- */
void FAR UpdateDateSamples(HWND hDlg);
void FAR UpdateTimeSample(HWND hDlg);
void FAR UpdateCurrencySamples(HWND hDlg);
void FAR UpdateNumberSample(HWND hDlg);

/* Прототипы диалоговых процедур */
BOOL WINAPI DateFmtDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
BOOL WINAPI TimeFmtDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
BOOL WINAPI NumberFmtDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
BOOL WINAPI CurrencyFmtDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);

#endif
