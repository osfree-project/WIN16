# osFree Janus Control Panel

![Language: C](https://img.shields.io/badge/language-C-blue)
![License: BSD-3-Clause](https://img.shields.io/badge/license-BSD--3--Clause-green)
![Platform: Win16](https://img.shields.io/badge/platform-Win16-lightgrey)
![Status: Alpha](https://img.shields.io/badge/status-alpha-red)

A clone of the classic Windows 3.x Control Panel (`CONTROL.EXE`).  
This is part of the [osFree Win16 Personality](https://github.com/osfree-project/WIN16) project — an open-source implementation of the 16-bit Windows environment.

## 📖 About

This application provides the main Control Panel window, loading and displaying any `.CPL` applets found in the system directory. It faithfully recreates the Windows 3.x Control Panel experience, including the `Settings` and `Help` menus, the iconic grid of applet icons, and the ability to launch individual configuration dialogs.

The project also includes a full set of standard applets inside `MAIN.CPL`. It is a component of the **osFree Janus** project to create an open-source clone of Windows 3.0.

## ✨ Features

- **Modular architecture** – Loads `.CPL` applets dynamically from the `SYSTEM` directory or from the `[MMCPL]` section of `CONTROL.INI`.
- **Standard applets included** – `MAIN.CPL` contains the core configuration applets:
  - Color
  - Fonts
  - Ports
  - Mouse
  - Desktop
  - Keyboard
  - Printers
  - International
  - Date/Time
  - Network
- **Interactive grid UI** – Displays applets as a grid of icons with labels.  
  Supports single-click selection and double-click to launch.
- **Full menu system** – Includes `File`, `Settings`, and `Help` menus, wired to launch the corresponding applets.
- **Scrollable view** – Supports both horizontal and vertical scrolling when applets exceed the window size.
- **User-friendly diagnostics** – Displays clear error messages when `.CPL` files cannot be loaded or are invalid.

## 🧩 Project Structure

| File | Description |
| :--- | :--- |
| `control.c` | Main Control Panel executable source |
| `control.h` | Header file with command identifiers |
| `control.rc` | Resource file (menus) |
| `makefile` | Build file for Open Watcom Make |
| `_wcc.cmd` / `_wcc.sh` | Auxiliary build scripts |
| `applets/main/` | Source for `MAIN.CPL` (standard applets) |
| `applets/main/main.c` | Main applet implementation |
| `applets/main/main.h` | Header file for the applet |
| `applets/main/main.rc` | Resource file for the applet (dialogs) |
| `applets/main/makefile` | Build file for `MAIN.CPL` |
| `applets/main/_wcc.cmd` / `_wcc.sh` | Auxiliary build scripts for the applet |
| `applets/cpl.txt` | Documentation on the CPL interface |

## 🤝 Contributing

We welcome your contributions! Please keep the following in mind:

- **Bug reports** – Create issues in the [Issues](https://github.com/osfree-project/control/issues) section.
- **Pull requests** – Send your improvements and fixes.
- **Documentation** – Help improve this README and other project documentation.

## 📜 License

This project is distributed under the **BSD 3-Clause License** (osFree license).  
You can find the full license text at the [osFree wiki](http://osfree.org/doku/doku.php?id=en:legal:osfree).

## 🔗 Related Projects

- [osFree Win16 Personality (WIN16)](https://github.com/osfree-project/WIN16) – the main project to create an open-source clone of Windows 3.x
- [osFree Project](https://github.com/osfree-project) – the parent project for an open-source OS/2 clone
- [winver](https://github.com/osfree-project/winver) – a clone of the About dialog
- [Notepad](https://github.com/osfree-project/notepad) – a clone of Notepad
- [Taskman](https://github.com/osfree-project/taskman) – a clone of Task Manager

## 👤 Copyright

- Copyright (C) 2026 Yuri Prokushev and the [osFree](https://github.com/osfree-project) team

---

*Last updated: May 26, 2026*
