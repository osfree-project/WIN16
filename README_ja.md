# osFree Janus: オープンソースのWindows 3.0クローン

<!-- hy-mt2-i18n:start -->
[English](./README.MD) | [中文](./README_zh-CN.md) | **日本語** | [Español](./README_es.md)
<!-- hy-mt2-i18n:end -->


![言語: C](https://img.shields.io/badge/language-C-blue)
![言語: アセンブリ](https://img.shields.io/badge/language-Asm-blue)
![ライセンス: LGPL-2.1](https://img.shields.io/badge/license-LGPL%202.1-green)
![ライセンス: GPL-2.1](https://img.shields.io/badge/license-GPL%202.1-green)
![ライセンス: BSD](https://img.shields.io/badge/license-BSD-green)
![プラットフォーム: DOS](https://img.shields.io/badge/platform-DOS-lightgrey)
![プラットフォーム: Win16](https://img.shields.io/badge/platform-Win16-lightgrey)
![ステータス: アルファ版](https://img.shields.io/badge/status-alpha-red)

**osFree Janus**は、古典的な16ビットMicrosoft Windows 3.0オペレーティングシステムを完全でオープンソースかつ互換性のある形で再実装するプロジェクトです。これはosFreeプロジェクトの一環として開発されており、単にカーネルだけでなくWindows 3.0環境全体を完全にコピーしたものです。

![Windows 3.0のクローンスクリーンショット](https://via.placeholder.com/640x480?text=osFree+Janus+Running+WinVer)

## 📖 概要

このプロジェクトの目標は、カーネル（KERNEL）、グラフィックスサブシステム（GDI、USER）、システムドライバ、およびコアコンポーネントを含む、歴史的なWindowsの独立した現代的な再実装を構築することであり、これにより「純粋な」DOS上でも仮想DOSマシン（VDM）セッション内でも単独で動作可能になります。

このプロジェクトは、VDMやスタンドアロン環境で利用できる16ビット版Windowsカーネルを再実装しようとする試みです。開発にあたっては、**TWIN**、**WINE**、**ReactOS**、**HX-DOS**といったプロジェクトのコードが幅広く活用されています。また、カーネルやシステム構造の再構築においては、**Matt Pietrek著「Windows Internals」**、**Andrew Schulman著「Undocumented Windows」**、**Daniel Norton著「Writing Windows Device Drivers」**といった書籍が大きく参考になっています。

## ✨ 機能概要

- **完全なWindows 3.0のクローン** – カーネルだけでなく、GDI、USER、ドライバもすべて含まれる  
- **複数環境対応** – 純粋なDOS上でもVDMセッション内でも動作する  
- **独立した再実装** – Win32やXFreeといった下層システムは不要  
- **KRNL286/KRNL386カーネル** – リアルモードのDOSから自律的に起動する  
- **既存のWin16アプリとの互換性** – 元のソフトウェアを実行できるように設計されている  
- **実績のあるオープンソースコードをベースに構築** – TWIN、Wine、ReactOS、HX-DOS

## 🏗️ 設計とアーキテクチャ

元のOS/2はほぼそのままのWindowsを使用していたため、このプロジェクトは従来の「純正」なWindows 3.xの再実装と見なすことができます。つまり、ほとんどのコードはWin32やXFreeといった下層システムなしでも動作するように書かれています。**KRNL286/386**カーネルは純粋なDOSから起動し、自律的に動作しなければなりません。

ほとんどの初期化コードおよびモジュールマネージャーは、実装の出発点となる**HX-DOS**から派生しています。

その他のほとんどのAPI関数は、純粋な16ビット環境への移植の難易度に応じて**Wine**や**TWIN**から採用されています。多くの関数は、純粋なDOS 16ビット環境向けに再移植されています。

## 📊 プロジェクトの進捗状況

このプロジェクトはまだ**極めて初期のアルファ開発段階**にあります。グラフィックスサブシステム（GDI）はまだ実装されていません。ユーザーインターフェースライブラリ（USER）には既に多くの動作可能な機能が備わっています。また、ほとんどの上位レベルのDLLもある程度実装されています。

**最初の目標**は、**WinVer**や**Clock**といったシンプルなシステムアプリケーションを正しく起動させることです。これには、カーネル（KERNEL）がドライバー（システム、マウス、キーボード）や**GDI.EXE**、**USER.EXE**を正しく読み込み、初期化する必要があります。

**ターゲットとなるWindowsバージョンは3.0です。**

## 🧩 プロジェクト構成

このプロジェクトには、Windows 3.0の主要なシステムコンポーネントすべての再実装が含まれています：

| Directory       | Description                                                                                       |
| :-------------- | :------------------------------------------------------------------------------------------------ |
| `applications`  | 標準のWindowsアプリケーション – TWIN、Wine、ReactOS、MS File Managerからのコンテンツが混在               |
| `resources`     | TWIN由来のアイコン、ビットマップ、カーソル、Wine由来のフォント                                   |
| `dlls`          | 標準のWindows DLL                                                                       |
| `docs`          | 配布用ドキュメント                                                                         |
| `dosx`          | 286 DPMIホストおよびエクステンダー（まだ実装されていない、基盤としてHXDOSが使用されている）            |
| `drivers`       | 標準のWindows/DOSドライバー                                                                  |
| `include`       | インクルードファイル（参照用のみ – OpenWatcomのヘッダーが使用されている）                     |
| `kernel`        | KERNEL.EXE / KRNL286.EXE / KRNL386.EXE — 新しいカーネル                                         |
| `MME`           | マルチメディア拡張機能                                                                        |
| `pal`           | プラットフォーム抽象レイヤー – X11ドライバー（使用されていない、参照用のみ）                    |
| `samples`       | 様々なサンプルプログラム                                                                    |
| `tests`         | WIN16単体テスト                                                                            |
| `user`          | USER.EXE                                                                                          |
| `utilities`     | ユーティリティプログラム                                                                     |
| `win`           | WIN.COM — Windowsローダー                                                                   |
| `winkrnl`       | TWINプロジェクト由来のWindowsカーネル、GDI、USER（参照用のみ – 現在のカーネルは`kernel/`にある） |

## 🤝 貢献の呼びかけ

ご貢献を心より歓迎します！以下の方法でお手伝いいただけます：

- 現在のアルファ版をテストし、バグを報告する  
- もっと多くのWine/TWIN関数を16ビット環境に移植する  
- 欠落している部分（GDI、ドライバなど）を実装する  
- ドキュメントやサンプルコードを作成する

[issue tracker](https://github.com/osfree-project/WIN16/issues) をご利用の上、メインリポジトリにプルリクエストを送信してください。

## 📜 ライセンス

**GNU Lesser General Public License v2.1 (LGPL‑2.1)** のもとで配布されています。詳細は [LICENSE](LICENSE) をご覧ください。

## 🔗 関連プロジェクト

- [osFree Project](https://github.com/osfree-project) – オープンソースのOS/2クローンを扱う親プロジェクト  
- [osFree Janus Clock](https://github.com/osfree-project/clock) – サンプルアプリケーション  
- [WinVer](https://github.com/osfree-project/winver) – 「About Windows」ダイアログ  
- [Notepad](https://github.com/osfree-project/notepad) – テキストエディタのクローン  
- [Taskman](https://github.com/osfree-project/taskman) – タスクマネージャのクローン

## 📌 リポジトリとキーワード

**プロジェクトリポジトリ:**  
[https://github.com/osfree-project/WIN16](https://github.com/osfree-project/WIN16)

**キーワード:**  
`Windows 3.0 clone` `Windows 3.0 implementation` `Windows 3.0 reimplementation`  
`Open-source Windows 3.0` `Win16` `KRNL286` `KRNL386` `16-bit Windows` `osFree`  
`Windows 3.0 compatibility` `Run old Windows programs` `DOS Windows`  
`Windows 3.0 repository`

## 👤 表彰と謝辞

- コードベースを提供してくれた **TWIN**、**WINE**、**ReactOS**、**HX-DOS** チームの皆様  
- 書籍を執筆してくれた **Matt Pietrek**、**Andrew Schulman**、**Daniel Norton** 様  
- osFree プロジェクトのすべての貢献者およびテスターの皆様

---

*最終更新日: 2026年6月10日*
