# backend.pri — Phase 2 backend sources (XdgPaths, DesktopFileHandler, MenuParser, MenuWriter)
# Include from shell/shell.pro with:
#   include($$PROJECT_ROOTDIR/backend/backend.pri)

BACKEND_DIR = $$PROJECT_ROOTDIR/backend

INCLUDEPATH += $$BACKEND_DIR

HEADERS += \
    $$BACKEND_DIR/xdgpaths.h \
    $$BACKEND_DIR/desktopfilehandler.h \
    $$BACKEND_DIR/menuparser.h \
    $$BACKEND_DIR/menuwriter.h

SOURCES += \
    $$BACKEND_DIR/xdgpaths.cpp \
    $$BACKEND_DIR/desktopfilehandler.cpp \
    $$BACKEND_DIR/menuparser.cpp \
    $$BACKEND_DIR/menuwriter.cpp

# libmenu-cache é usado para leitura do menu XDG de aplicações
CONFIG  += link_pkgconfig
PKGCONFIG += libmenu-cache
