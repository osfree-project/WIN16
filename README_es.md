# osFree Janus: Clon de código abierto de Windows 3.0

<!-- hy-mt2-i18n:start -->
[English](./README.MD) | [中文](./README_zh-CN.md) | [日本語](./README_ja.md) | **Español**
<!-- hy-mt2-i18n:end -->


![Idioma: C](https://img.shields.io/badge/language-C-blue)
![Idioma: Asambleador](https://img.shields.io/badge/language-Asm-blue)
![Licencia: LGPL-2.1](https://img.shields.io/badge/license-LGPL%202.1-green)
![Licencia: GPL-2.1](https://img.shields.io/badge/license-GPL%202.1-green)
![Licencia: BSD](https://img.shields.io/badge/license-BSD-green)
![Plataforma: DOS](https://img.shields.io/badge/platform-DOS-lightgrey)
![Plataforma: Win16](https://img.shields.io/badge/platform-Win16-lightgrey)
![Estado: Alfa](https://img.shields.io/badge/status-alpha-red)

**osFree Janus** es un proyecto cuyo objetivo es crear una implementación completa, de código abierto y compatible del clásico sistema operativo de 16 bits Microsoft Windows 3.0. Se trata de una copia total del entorno de Windows 3.0, y no solo de su kernel, desarrollada como parte del proyecto osFree.

![Captura de pantalla del clon de Windows 3.0](https://via.placeholder.com/640x480?text=osFree+Janus+Running+WinVer)

## 📖 Acerca de

El objetivo es crear una reimplementación independiente y moderna del antiguo Windows, que incluya el kernel (KERNEL), el subsistema gráfico (GDI, USER), los controladores del sistema y los componentes esenciales, capaz de ejecutarse tanto de forma autónoma en un entorno “puro” de DOS como dentro de una sesión de Virtual DOS Machine (VDM).

Este proyecto representa un intento por volver a implementar el kernel de Windows de 16 bits para su uso en entornos VDM y de forma independiente. Su desarrollo hace amplio uso del código proveniente de los proyectos **TWIN**, **WINE**, **ReactOS** y **HX-DOS**. La reconstrucción de la estructura interna del kernel y del sistema se basa en gran medida en los libros **“Windows Internals” de Matt Pietrek**, **“Undocumented Windows” de Andrew Schulman** y **“Writing Windows Device Drivers” de Daniel Norton**.

## ✨ Características

- **Clon completo de Windows 3.0**: no solo el kernel, sino también GDI, USER y los controladores completos.  
- **Soporte para entornos duales**: funciona tanto en DOS puro como dentro de una sesión de VDM.  
- **Reimplementación independiente**: no se requiere ningún sistema subyacente como Win32 o XFree.  
- **Kernels KRNL286/KRNL386**: arrancan de forma autónoma desde DOS en modo real.  
- **Compatibilidad con aplicaciones Win16 existentes**: tiene como objetivo ejecutar el software original.  
- **Desarrollado a partir de bases de código abierto probadas**: TWIN, Wine, ReactOS, HX-DOS.

## 🏗️ Diseño y arquitectura

El OS/2 original utilizaba una versión de Windows prácticamente sin modificaciones, por lo que este proyecto puede considerarse como una reimplementación del clásico Windows 3.x “puro”. Esto implica que la mayor parte del código está diseñado para ejecutarse sin necesidad de ningún sistema subyacente (como Win32 o XFree). Los kernels **KRNL286/386** deben iniciarse desde DOS puro y operar de forma autónoma.

La mayor parte del código de inicialización y el gestor de módulos provienen de **HX-DOS**, que funciona como la base de implementación inicial.

La mayoría de las demás funciones de la API provienen de **Wine** y **TWIN**, según el grado de dificultad para su adaptación a un entorno puramente de 16 bits. Muchas funciones han sido trasladadas nuevamente a un entorno puramente de DOS de 16 bits.

## 📊 Estado del proyecto

El proyecto se encuentra en una **etapa muy temprana de desarrollo alfa**. El subsistema gráfico (GDI) aún no está implementado. La biblioteca de interfaz de usuario (USER) ya cuenta con muchas funcionalidades operativas. La mayoría de los DLL de nivel superior también están implementados, aunque de forma limitada.

**El primer objetivo** es ejecutar correctamente una aplicación de sistema sencilla, como **WinVer**, **Clock**, entre otras. Para ello, el kernel (KERNEL) debe cargar e inicializar adecuadamente los controladores (del sistema, del ratón, del teclado), así como **GDI.EXE** y **USER.EXE**.

La versión de Windows objetivo es 3.0.

## 🧩 Estructura del proyecto

El proyecto incluye nuevas implementaciones de todos los componentes clave del sistema Windows 3.0:

| Directorio       | Descripción                                                                                       |
| :-------------- | :------------------------------------------------------------------------------------------------ |
| `applications`  | Aplicaciones estándar de Windows: una mezcla de TWIN, Wine, ReactOS y el Gestor de Archivos de MS        |
| `resources`     | Iconos, mapas de bits y cursores de TWIN; fuentes de Wine                                               |
| `dlls`          | DLLs estándar de Windows                                                                             |
| `docs`          | Documentación para la distribución                                                                    |
| `dosx`          | Anfitrión y extensor DPMI para 286 (aún no implementado; se utiliza HXDOS como base)                           |
| `drivers`       | Controladores estándar de Windows/DOS                                                                      |
| `include`       | Archivos de inclusión (solo de referencia; se utilizan los encabezados de OpenWatcom)                     |
| `kernel`        | KERNEL.EXE / KRNL286.EXE / KRNL386.EXE: el nuevo núcleo                                                  |
| `MME`           | Extensiones multimedia                                                                             |
| `pal`           | Capa de abstracción de plataforma: controladores X11 (no se utilizan, solo de referencia)              |
| `samples`       | Varios programas de ejemplo                                                                           |
| `tests`         | Pruebas unitarias WIN16                                                                                  |
| `user`          | USER.EXE                                                                                          |
| `utilities`     | Programas utilitarios                                                                                |
| `win`           | WIN.COM: cargador de Windows                                                                          |
| `winkrnl`       | Núcleo de Windows, GDI y USER del proyecto TWIN (solo de referencia; el núcleo actual se encuentra en `kernel/`) |

## 🤝 Contribuciones

¡Aceptamos contribuciones! Puede ayudarnos de la siguiente manera:

- Probar la versión alfa actual e informar sobre errores  
- Portar más funciones de Wine/TWIN al entorno de 16 bits  
- Implementar las partes faltantes (GDI, controladores, etc.)  
- Redactar documentación y ejemplos

Por favor, utilice el [rastreador de problemas](https://github.com/osfree-project/WIN16/issues) y envíe solicitudes de pull request al repositorio principal.

## 📜 Licencia

Distribuido bajo la **Licencia Pública General Menor de GNU v2.1 (LGPL‑2.1)**.  
Consulte [LICENSE](LICENSE) para más detalles.

## 🔗 Proyectos relacionados

- [osFree Project](https://github.com/osfree-project) – proyecto padre de un clon de OS/2 de código abierto  
- [osFree Janus Clock](https://github.com/osfree-project/clock) – aplicación de ejemplo  
- [WinVer](https://github.com/osfree-project/winver) – cuadro de diálogo “Acerca de Windows”  
- [Notepad](https://github.com/osfree-project/notepad) – clon de un editor de texto  
- [Taskman](https://github.com/osfree-project/taskman) – clon de un gestor de tareas

## 📌 Repositorio y palabras clave

**Repositorio del proyecto:**  
[https://github.com/osfree-project/WIN16](https://github.com/osfree-project/WIN16)

**Palabras clave:**  
`Clon de Windows 3.0` `Implementación de Windows 3.0` `Reimplementación de Windows 3.0`  
`Windows 3.0 de código abierto` `Win16` `KRNL286` `KRNL386` `Windows de 16 bits` `osFree`  
`Compatibilidad con Windows 3.0` `Ejecutar programas antiguos de Windows` `Windows basado en DOS`  
`Repositorio de Windows 3.0`

## 👤 Agradecimientos

- Los equipos de **TWIN**, **WINE**, **ReactOS** y **HX-DOS** por sus bases de código  
- **Matt Pietrek**, **Andrew Schulman** y **Daniel Norton** por sus libros  
- Todos los colaboradores y probadores del proyecto osFree

---

*Última actualización: 10 de junio de 2026*
