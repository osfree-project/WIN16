@echo off
@REM SPDX-FileCopyrightText: (C) 2026 osFree Project
@REM SPDX-License-Identifier: LGPL-2.1-or-later
rem set VERBOSE=yes
set root=.
:loop
if exist "%root%\tools\mk\all.mk" goto found
set root=%root%\..
goto loop
:found
set path=%root%\tools\conf\scripts;%path%
call build %1 %2 %3 %4 %5 %6 %7 %8 %9
