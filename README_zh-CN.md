# osFree Janus：开源版 Windows 3.0 复刻版

<!-- hy-mt2-i18n:start -->
[English](./README.MD) | **中文** | [日本語](./README_ja.md) | [Español](./README_es.md)
<!-- hy-mt2-i18n:end -->


![语言：C](https://img.shields.io/badge/language-C-blue)
![语言：汇编](https://img.shields.io/badge/language-Asm-blue)
![许可证：LGPL-2.1](https://img.shields.io/badge/license-LGPL%202.1-green)
![许可证：GPL-2.1](https://img.shields.io/badge/license-GPL%202.1-green)
![许可证：BSD](https://img.shields.io/badge/license-BSD-green)
![平台：DOS](https://img.shields.io/badge/platform-DOS-lightgrey)
![平台：Win16](https://img.shields.io/badge/platform-Win16-lightgrey)
![状态：测试版](https://img.shields.io/badge/status-alpha-red)

**osFree Janus** 是一个旨在打造经典16位微软Windows 3.0操作系统的完整、开源且兼容的重新实现项目的作品。作为osFree项目的一部分，它并非仅仅复制该操作系统的内核，而是对整个Windows 3.0环境进行全面克隆。

![Windows 3.0 克隆版截图](https://via.placeholder.com/640x480?text=osFree+Janus+Running+WinVer)

## 📖 关于项目

该项目的目标是打造一个独立且现代化的经典Windows系统重实现版本，涵盖内核（KERNEL）、图形子系统（GDI、USER）、系统驱动以及核心组件，使其既能在“纯”DOS环境下独立运行，也能在虚拟DOS机（VDM）环境中运行。

该项目旨在重新实现适用于虚拟磁盘机及独立运行环境的16位Windows内核。其开发过程中大量借鉴了**TWIN**、**WINE**、**ReactOS**以及**HX-DOS**项目的代码。而内核与系统内部结构的重构则主要参考了**Matt Pietrek所著的《Windows Internals》**、**Andrew Schulman所著的《Undocumented Windows》**以及**Daniel Norton所著的《Writing Windows Device Drivers》**这三本著作。

## ✨ 功能特性

- **完整的Windows 3.0克隆版**——不仅是内核，还包括完整的GDI、USER组件以及各类驱动程序  
- **双环境支持**——既可在纯DOS环境下运行，也可在虚拟DOS机（VDM）会话中运行  
- **独立重实现**——无需依赖任何底层的Win32或XFree系统  
- **KRNL286/KRNL386内核**——可从实模式DOS直接自主启动  
- **兼容现有Win16应用程序**——旨在能够运行原始软件  
- **基于经过验证的开源项目构建**——TWIN、Wine、ReactOS、HX-DOS

## 🏗️ 设计与架构

最初的OS/2基于几乎未做修改的Windows开发，因此该项目可被视为经典“原生”Windows 3.x的重新实现。这意味着大部分代码都被设计为无需任何底层系统（如Win32或XFree）即可运行。**KRNL286/386**内核必须从纯DOS环境启动，并能够独立运作。

大部分初始化代码以及模块管理器均源自作为初始实现基础的**HX-DOS**。

其余大部分 API 函数则根据移植到纯 16 位环境时的难度，分别取自 **Wine** 和 **TWIN**；同时也有许多函数已被重新移植回纯 DOS 16 位环境。

## 📊 项目现状

该项目目前仍处于**极为早期的alpha开发阶段**。图形子系统（GDI）尚未实现，而用户界面库（USER）已具备大量可用的功能。大多数高级DLL也在一定程度上实现了相应功能。

**首要目标**是能够正确启动诸如**WinVer**、**Clock**之类的简单系统应用程序。这要求内核（KERNEL）能够正确加载并初始化各类驱动程序（系统驱动、鼠标驱动、键盘驱动），以及**GDI.EXE**和**USER.EXE**。

该项目的目标 Windows 版本为 3.0。

## 🧩 项目结构

该项目包含对所有关键的 Windows 3.0 系统组件的重新实现：

| 目录             | 描述                                                                                         |
| :--------------- | :------------------------------------------------------------------------------------------- |
| `applications`  | 标准 Windows 应用程序——整合了来自 TWIN、Wine、ReactOS 以及 MS 文件管理器的程序            |
| `resources`     | 来自 TWIN 的图标、位图和光标，以及来自 Wine 的字体                                            |
| `dlls`          | 标准 Windows DLL 文件                                                                              |
| `docs`          | 发行相关文档                                                                                    |
| `dosx`          | 286 DPMI 主机及扩展模块（尚未实现，目前以 HXDOS 作为基础）                                    |
| `drivers`       | 标准 Windows/DOS 驱动程序                                                                        |
| `include`       | 包含文件（仅作参考——实际使用的是 OpenWatcom 的头文件）                                      |
| `kernel`        | KERNEL.EXE / KRNL286.EXE / KRNL386.EXE —— 新内核                                                |
| `MME`           | 多媒体扩展功能                                                                                  |
| `pal`           | 平台抽象层——X11 驱动程序（未实际使用，仅作参考）                                          |
| `samples`       | 各种示例程序                                                                                    |
| `tests`         | WIN16 单元测试                                                                                  |
| `user`          | USER.EXE                                                                                      |
| `utilities`     | 实用工具程序                                                                                  |
| `win`           | WIN.COM —— Windows 加载器                                                                        |
| `winkrnl`       | 来自 TWIN 项目的 Windows 内核、GDI 和 USER 组件（仅作参考——当前内核位于 `kernel/` 目录中） |

## 🤝 参与贡献

我们欢迎大家的贡献！您可以通过以下方式提供帮助：

- 测试当前的测试版并报告漏洞  
- 将更多 Wine/TWIN 函数移植到 16 位环境中  
- 实现缺失的功能模块（如 GDI、驱动程序等）  
- 编写文档与示例代码

请使用[问题追踪器](https://github.com/osfree-project/WIN16/issues)，并将拉取请求提交到主仓库中。

## 📜 许可证

本作品依据 **GNU 较宽松通用公共许可证 v2.1 (LGPL‑2.1)** 发布。
详情请参阅 [LICENSE](LICENSE)。

## 🔗 相关项目

- [osFree Project](https://github.com/osfree-project) – 一个开源 OS/2 克隆版本的母项目
- [osFree Janus Clock](https://github.com/osfree-project/clock) – 示例应用
- [WinVer](https://github.com/osfree-project/winver) – “关于 Windows”对话框
- [Notepad](https://github.com/osfree-project/notepad) – 文本编辑器克隆版
- [Taskman](https://github.com/osfree-project/taskman) – 任务管理器克隆版

## 📌 仓库与关键词

**项目仓库地址：**  
[https://github.com/osfree-project/WIN16](https://github.com/osfree-project/WIN16)

**关键词：**  
`Windows 3.0 克隆版` `Windows 3.0 实现` `Windows 3.0 重实现`  
`开源 Windows 3.0` `Win16` `KRNL286` `KRNL386` `16位 Windows` `osFree`  
`Windows 3.0 兼容性` `运行旧版 Windows 程序` `DOS Windows`  
`Windows 3.0 项目仓库`

## 👤 致谢

- 提供代码库的 **TWIN**、**WINE**、**ReactOS** 以及 **HX-DOS** 团队
- 撰写相关书籍的 **Matt Pietrek**、**Andrew Schulman**、**Daniel Norton**
- osFree 项目的所有贡献者和测试人员

---

*最后更新时间：2026年6月10日*
