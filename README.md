# MenuLibre
**MenuLibre** is an advanced FreeDesktop.org compliant menu editor.

![MenuLibre window](https://github.com/bluesabre/openkylinmenueditor/blob/wiki-assets/readme/openkylinmenueditor-github.png)

All fields specified in the FreeDesktop.org [Desktop Entry](https://specifications.freedesktop.org/desktop-entry-spec/latest/) and [Menu](https://specifications.freedesktop.org/menu-spec/latest/) 
specifications are available to quickly update.
Additionally, MenuLibre provides an editor for the launcher actions 
used by applications such as Unity and Plank.

## Features
 - Modern and high-performance interface powered by Qt (C++)
 - Create or modify launchers with full FreeDesktop.org specification support
 - Add, remove, and adjust desktop actions for advanced integration
 - Easily rearrange menu items to suit your needs

### Build Requirements
 - Qt5/Qt6 development libraries
 - C++17 compatible compiler
 - intltool
 - libgnome-menu-3-dev

### Runtime Requirements
 - Qt
 - gnome-menus
 - xdg-utils


## Installation

1. Install the build requirements above
2. Compile via CMake (recomendado):
   mkdir build && cd build
   cmake ..
   make

   # Se o Qt não estiver no caminho padrão:
   # cmake .. -DCMAKE_PREFIX_PATH=/caminho/para/Qt/6.x.x/gcc_64

3. (Alternativo, build legado via qmake):
   cd shell && qmake && make

## Note:
Esta versão é apenas C++/Qt. Todo o código Python foi removido do projeto.

## Links
 - [Homepage](https://bluesabre.org/openkylinmenueditor/)
 - [Releases](https://github.com/bluesabre/openkylinmenueditor/releases)
 - [Bug Reports](https://github.com/bluesabre/openkylinmenueditor/issues)
 - [Translations](https://www.transifex.com/bluesabreorg/openkylinmenueditor)
 - [Wiki](https://github.com/bluesabre/openkylinmenueditor/wiki)
