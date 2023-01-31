This is a Program Manager for WINE.

There is a checksum in the Microsoft `*.grp' files. I don't know how
to calculate it (1). Therefore the group files written by this Program Manager
cannot be used with the Microsoft Program Manager!!

To prevent overwriting original files:
If there is an existing `*.grp' file this program uses the extension
`.gr' instead.

It's possible to use an alternate `progman.ini' file by adding to
`wine.conf' something like:
[progman]
progman.ini=/my/wine/path/progman.ini

1) Information about checksum calculation can be found in https://books.google.ru/books?id=LIyy_CtozLcC&lpg=PP1&hl=ru&pg=PP2#v=onepage&q&f=false PCMag 12 Jan 1993


https://www.moon-soft.com/program/format/windows/grp.htm
http://old-dos.ru/books/c/8/c/ComputerPress-1992-06.pdf
