PROJ = win

# Явно переопределяем автоопределение (каталог в DOS/WIN16/ дал бы TARGET_API=WIN)
TARGET_API = DOS
TARGET_CLASS = APPLICATION
TARGET_SUBCLASS = CONSOLE

# .COM-формат + расширение .cnf
COM = 1
EXT = cnf
TARGET_EXT = cnf
JWASM = 1
NOLIBS     = 1

# Установка в WINOS2 после сборки
DEST       = os2$(SEP)mdos$(SEP)winos2$(SEP)system

# Регистрация в setup-пакете d1
TARGET_PACKAGE = DISK01

# Макробиблиотека (bios.inc, dos.inc из execlogo.asm)
ADD_ASMOPT = -I=$(MYDIR)..$(SEP)..$(SEP)macrolib

!include $(%ROOT)\tools\mk\build.mk
