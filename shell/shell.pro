#-------------------------------------------------
#
# OpenKylin Menu Editor - Shell
# Qt5/C++ rewrite of the Python/GTK original
#
#-------------------------------------------------

QT += core gui widgets xml dbus

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET   = openkylinmenueditor
DESTDIR  = ../bin
TEMPLATE = app

DEFINES += QT_DEPRECATED_WARNINGS

include(../env.pri)
include($$PROJECT_ROOTDIR/libukmge/interface.pri)
include($$PROJECT_ROOTDIR/libukmge/widgets/FlowLayout/flowlayout.pri)
include($$PROJECT_ROOTDIR/backend/backend.pri)

CONFIG += link_pkgconfig c++11
PKGCONFIG += gio-2.0 gio-unix-2.0 gsettings-qt
LIBS += -L$$PROJECT_ROOTDIR/libukmge -lukmge
INCLUDEPATH += $$PROJECT_ROOTDIR/libukmge/widgets/CloseButton

desktop.files += openkylinmenueditor.desktop
desktop.path   = /usr/share/applications

modules.files += $$PROJECT_ROOTDIR/data/openkylinmenueditor-config.json
modules.path  += /usr/share/openkylinmenueditor/data/

target.source += $$TARGET
target.path    = /usr/bin

INSTALLS += target desktop modules

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    titlewidget.cpp \
    leftsidebarwidget.cpp \
    mstackwidget.cpp \
    menutreewidget.cpp \
    applicationeditorwidget.cpp \
    editorwidget.cpp \
    searchbar.cpp

HEADERS += \
    mainwindow.h \
    titlewidget.h \
    leftsidebarwidget.h \
    mstackwidget.h \
    menutreewidget.h \
    applicationeditorwidget.h \
    editorwidget.h \
    searchbar.h

# Interface manual: sem arquivos .ui, C++ puro

RESOURCES += \
    res/resfile.qrc

TRANSLATIONS += \
    res/i18n/en_US.ts \
    res/i18n/pt_BR.ts
