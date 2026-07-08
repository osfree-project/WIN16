```markdown
# osFree Win16 NLS API Library

![Language: C](https://img.shields.io/badge/language-C-blue)
![License: BSD-3-Clause](https://img.shields.io/badge/license-BSD--3--Clause-green)
![Platform: Win16](https://img.shields.io/badge/platform-Win16-lightgrey)
![Status: Beta](https://img.shields.io/badge/status-beta-yellow)

A Win32‑compatible National Language Support layer for 16‑bit Windows.  
Part of the [osFree Win16 Personality](https://github.com/osfree-project/WIN16) project — an open‑source implementation of the 16‑bit Windows environment.

## 📖 About

This library implements a substantial subset of the **Win32 NLS API** directly on top of Windows 3.0 mechanisms.  
It provides the familiar `GetDateFormat`, `GetTimeFormat`, `GetNumberFormat`, `GetCurrencyFormat`, `GetLocaleInfo` and other functions, using the data found in `WIN.INI` and `SETUP.INF`.  

The sole purpose of the library is to **allow applications to use a Win32‑style programming interface while still running under Windows 3.x**.  
When such an application is later recompiled for a real 32‑bit environment, **no source‑code changes are required** – the calls are simply resolved against the system’s native NLS API.

## ✨ Features

- **Familiar Win32 signatures** – all functions mirror their 32‑bit counterparts (`GetDateFormatA`, `GetTimeFormatA`, …).
- **Locale data from real Windows 3.0 sources** – user settings are read from `[intl]` section of `WIN.INI`; supplementary locale data is parsed from `SETUP.INF`.
- **Full formatting pipeline** – date, time, number and currency formatting with configurable separators, digit grouping, negative patterns, and currency placement.
- **Enumeration of available locales** – `EnumSystemLocales`, `EnumUILanguages`, `EnumDateFormats`, `EnumTimeFormats`, `EnumCalendarInfo`.
- **Read‑write access to locale settings** – `SetLocaleInfo` writes back to `WIN.INI` (behaving exactly as Windows 3.x does).
- **Thread / system locale stubs** – `GetThreadLocale`, `GetSystemDefaultLCID`, `GetUserDefaultLCID`, making source code compatible with Win32 conventions.
- **Built‑in LCID mapping** – a table maps country codes and language abbreviations to standard LCID values (0x0409, 0x040C, …), with a fallback heuristic for unknown locales.

## 📋 Supported API

| Win32 function | Implementation status | Remarks |
| :--- | :--- | :--- |
| `GetDateFormatA` | ✔ | Supports `DATE_SHORTDATE` / `DATE_LONGDATE` and custom picture strings. |
| `GetTimeFormatA` | ✔ | Supports 12/24‑hour format, minutes/seconds suppression, AM/PM markers. |
| `GetNumberFormatA` | ✔ | Rounding, digit grouping, five negative patterns (0–4). |
| `GetCurrencyFormatA` | ✔ | All 16 negative orders and 4 positive orders as documented by Microsoft. |
| `GetLocaleInfoA` | ✔ | Covers ~30 essential `LCTYPE` values, including day/month names, separators, currency symbol, number formats. |
| `SetLocaleInfoA` | ✔ | Writes to `[intl]` section of `WIN.INI`. |
| `EnumSystemLocalesA` | ✔ | Iterates `[country]` entries of `SETUP.INF`. |
| `EnumUILanguagesA` | ✔ | Reads `[language]` section; supports `MUI_LANGUAGE_ID` and `MUI_LANGUAGE_NAME`. |
| `EnumDateFormatsA` | ✔ | Returns short and long date formats for the given locale. |
| `EnumTimeFormatsA` | ✔ | Constructs time format from locale settings. |
| `EnumCalendarInfoA` | ✔ | Only Gregorian calendar; provides name, day names, date formats. |
| `GetThreadLocale` / `SetThreadLocale` | ✔ | Stub: `Get` returns `LOCALE_USER_DEFAULT`, `Set` always fails (thread locales are not a Win16 concept). |
| `GetSystemDefaultLCID` / `GetUserDefaultLCID` | ✔ | Return `LOCALE_SYSTEM_DEFAULT` / `LOCALE_USER_DEFAULT`. |
| `GetKeyboardLayoutList` | ✔ (custom) | Returns list of keyboard layout names from `SETUP.INF`. Note: Win32 prototype differs, requiring minor adaptation when porting. |
| `CompareString`, `LCMapString`, `GetCPInfo`, `MultiByteToWideChar`, etc. | ✘ | Deliberately omitted – they either require Unicode or rely on advanced linguistic data unavailable in Windows 3.0. |

## 🧩 Project Structure

| File | Description |
| :--- | :--- |
| `winnls.h` | Public header — defines types (`LCID`, `LCTYPE`, `SYSTEMTIME`, …) and function prototypes. |
| `nlsapi_internal.h` | Internal header — helper functions, `DECLSPEC` macro, `KNOWN_LCID` structure. |
| `nlsapi_main.c` | Core implementation: `GetLocaleInfo`, `SetLocaleInfo`, `GetDateFormat`, `GetTimeFormat`, locale/thread stubs. |
| `nlsapi_num.c` | Number formatting (`GetNumberFormatA`) with grouping, rounding and negative patterns. |
| `nlsapi_currency.c` | Currency formatting (`GetCurrencyFormatA`) with full negative/positive order tables. |
| `nlsapi_locale.c` | LCID lookup table, `GetLocaleInfoFromInf` (parses `SETUP.INF`), `GetKeyboardLayoutList`. |
| `nlsapi_enum.c` | Enumeration functions: `EnumSystemLocales`, `EnumUILanguages`, `EnumDateFormats`, `EnumTimeFormats`, `EnumCalendarInfo`. |
| `nlsapi_utils.c` | Utility functions: `OpenSetupInf`, `ParseCountryLine`, `AtoiFar`, `StringCopyN`. |

## ⚙️ Architecture

The library bridges two worlds:  
- **Win32‑style front end** — all exported functions accept `LCID`, `DWORD` flags, `SYSTEMTIME` structures and behave as closely as possible to the documented Win32 semantics.  
- **Win16 back end** — locale data is fetched from the traditional Windows 3.0 sources:
  - `WIN.INI` section `[intl]` (user preferences)
  - `SETUP.INF` section `[country]` (detailed locale definitions)

A lookup table (`knownLCIDs`) translates a Win32 LCID into a pair `(country code, language abbreviation)` that is then used to locate the appropriate record in `SETUP.INF`. This allows the library to serve locale information for **any locale present in the INF file**, not just the current user locale.

All static variables are allocated on function‑level (no global state), making the library suitable for the single‑threaded Windows 3.x environment.

## ⚠️ Limitations

The library deliberately covers only the **most commonly used NLS features** – the ones required by typical business applications for formatting and locale discovery.

- **No Unicode support** – all strings are ANSI (8‑bit). Functions like `MultiByteToWideChar` or `LCMapString` are not provided.
- **No string comparison / sorting** – `CompareString` is absent; applications must rely on C‑runtime or the basic `lstrcmp` / `lstrcmpi` for collation.
- **No code‑page enumeration** – `GetCPInfo`, `EnumSystemCodePages` are not implemented.
- **No advanced calendar support** – only the Gregorian calendar is handled.
- **Number grouping is single‑level** – complex grouping schemes (e.g., `"3;2;0"`) are not supported; only the primary group size is applied.
- **Thread locale is a stub** – `SetThreadLocale` always fails because Win16 does not support per‑thread locale.

These omissions are **by design**. They reflect the capabilities of the underlying 16‑bit platform. When the same source code is compiled for Win32, all missing features become available through the operating system’s native NLS API.

## 🤝 Contributing

We welcome your contributions! Please keep the following in mind:

- **Bug reports** – create issues in the [Issues](https://github.com/osfree-project/WIN16/issues) section of the WIN16 repository.
- **Pull requests** – send improvements and fixes against the `master` branch.
- **Documentation** – help improve this README and other project documentation.

## 📜 License

This project is distributed under the **BSD 3‑Clause License** (osFree license).  
You can find the full license text at the [osFree wiki](http://osfree.org/doku/doku.php?id=en:legal:osfree).

## 🔗 Related Projects

- [osFree Win16 Personality (WIN16)](https://github.com/osfree-project/WIN16) – the main project to create an open‑source clone of Windows 3.x
- [osFree Project](https://github.com/osfree-project) – the parent project for an open‑source OS/2 clone
- [osFree Control Panel](https://github.com/osfree-project/control) – a clone of the classic Windows 3.x Control Panel
- [winver](https://github.com/osfree-project/winver) – a clone of the About dialog
- [Notepad](https://github.com/osfree-project/notepad) – a clone of Notepad
- [Taskman](https://github.com/osfree-project/taskman) – a clone of Task Manager

## 👤 Copyright

- Copyright (C) 2026 Yuri Prokushev and the [osFree](https://github.com/osfree-project) team

---

*Last updated: July 8, 2026*
```
