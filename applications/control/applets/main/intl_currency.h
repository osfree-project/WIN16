/*
 * currency.h - определения для диалога Currency Format
 */

#ifndef _CURRENCY_H_
#define _CURRENCY_H_

#define DLG_CURRFMT              18   // ID ресурса диалога
#define IDC_CURRFMT_PLACEMENT    270  // COMBOBOX размещения символа
#define IDC_CURRFMT_NEGATIVE     274  // COMBOBOX отрицательного формата
#define IDC_CURRFMT_SYMBOL_EDIT  275  // EDIT поля символа валюты
#define IDC_CURRFMT_DIGITS_EDIT  276  // EDIT поля количества знаков

BOOL WINAPI CurrencyFmtDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);

#endif /* _CURRENCY_H_ */
