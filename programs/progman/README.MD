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


## PROGMAN.INI: The Program Manager Initialization File

The Program Manager initialization file, PROGMAN.INI, usually has entries for
[settings] and [groups], which describe what should appear in the Program
Manager window when you run Program Manager. To change the appearance
and behavior of Program Manager, use the menu commands as described in
Chapter 3, "Program Manager," in the Windows User's Guide.

A system administrator might also choose to add a third section, [restrictions],
for custom installations. For more information about creating custom entries
for PROGMAN.INI for multiple systems during Windows installation, see
"Configuring Program Manager for a Network Workgroup" in Chapter 6,
"Tips for Configuring Windows 3 .1."

[settings}

The [settings] section usually has these entries:

[Settings]
Window=60 28 565 388 1
SaveSettings=1
MinOnRun=O
AutoArrange=1
Startup=

Entry			Value

Window=         Four numbers that indicate the position of the window when Program Manager
				is opened, followed by a 1 if the window is maximized.

SaveSettings=	1 if the Save Settings On Exit conunand on the Options menu is checked in
				Program Manager. In this case, Program Manager will save the current
				configuration when you close Windows.
	
MinOnRun=		1 if the Minimize On Use command is checked on the Options menu in Program
				Manager. In this case, the Program Manager will be iconized when you run
				another application.

AutoArrange=	1 if the AutoAtrange command is checked on the Options menu in Program
				Manager. In this case, the icons in each group will be automatically arranged
				when you run Program Manager.

Startup=		Name of the group that serves as the "startup" group. If this entry is blank, the
				Startup group created in Windows Setup is the startup group.


[groups]

The PROGMAN.INI file will also have a section named [groups], which has
entries such as these:

[Groups]
Group1=C:\WINDOWSIACCESSOR.GRP
Group2=C:\WINDOWS\GAMES.GRP
Group3=C:\WINDOWSIALDUS.GRP
Group5=C:\WINDOWS\WORDFORW.GRP
Group6=C:\WINDOWS\MAIN.GRP
Group7=C:\WINDOWS\STARTUP.GRP
Order=8 7 2 3 5 1 6
Group8=C:\WINDOWSIAPPLICAT.GRP

Entry		Value

Groupx=		A filename (with full path) for the .GRP file that contains the group created
			during Setup, or created when an application was installed, or that you created in
			Program Manager.

Order=		A list of numbers separated with spaces, indicating the order in which the groups
			are drawn in the window.
			
To change these settings, we recommend that you use the commands on the
Program Manager menu.

[restrictions]

For network system administration, you can also add a [restrictions] section to
PROGMAN.INI to restrict user actions. The [restrictions] section can have
these entries:

[Restrictions]
NoRun=
NoClose=
NoSaveSettings=
NoFileMenu=
Editlevel=

Entry				Value

NoRun=				1 disables the Run command on the File menu. The Run command will appear
					dimmed on the File menu and the user will not be able to run applications from
					Program Manager unless the applications are set up as icons in a group.
			
NoClose=			1 disables the Exit Windows command on the File menu. Users will not be able
					to quit Program Manager through the File Menu or the Control menu (the Exit
					Windows and Close commands will be dimmed), or by using ALT+F4.
	
NoSaveSettlngs=		1 disables the Save Settings on Exit command on the Options menu. The Save
					Settings command will appear dimmed on the Options menu and any changes
					that the user makes to the arrangement of windows and icons will not be saved
					when Windows is restarted. This setting overrides the SaveSettings= entry in the
					[Settings] section of the PROGMAN.INI file.

NoFlleMenu=			1 removes the File menu from Program Manager. All of the commands on that
					menu will be unavailable. Users can start the applications in groups by selecting
					them and pressing ENTER, or by double-clicking the icon. Unless you have also
					disabled the Exit Windows command, users can still quit Windows by using the
					Control menu or ALT+F4.
					
EditLevel=n			Sets restrictions for what users can modify in Program Manager. You can specify
					one of the following values for n:

					0 allows the user to make any change. (This is the default value.)

					1 prevents the user from creating, deleting, or renaming groups. If you specify
					this value, the New, Move, Copy, and Delete commands on the File menu are
					not available when a group is selected.

					2 sets all restrictions in EditLevel=l, plus prevents the user from creating or
					deleting program items. If you specify this value, the New, Move, Copy, and
					Delete commands on the File menu are not available at all.

					3 sets all restrictions in EditLevel=2, plus prevents the user from changing
					command lines for program items. If you specify this value, the text in the
					Command Line box in the Properties dialog box cannot be changed.

					4 sets all restrictions in EditLevel=3, plus prevents the user from changing any
					program item information. If you specify this value, none of the areas in the
					Properties dialog box can be modified. The user can view the dialog box, but all
					of the areas are dimmed.
					
					To enable any of the commands or remove any of the EditLevel= restrictions,
					either remove the entry from the PROGMAN.INI file, or set the value to 0.

## Build defines

RESTRICTED - produce restricted version (also known as PROGMANR.EXE)
DEBUG - produce debug version (output debug information to debugger)
