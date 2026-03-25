QT += core widgets dbus

TARGET   = ukmge
TEMPLATE = lib
DEFINES += LIBUKMGE_LIBRARY

CONFIG += link_pkgconfig c++11

PKGCONFIG += gio-2.0 gio-unix-2.0 gsettings-qt
QMAKE_CXXFLAGS += -I/usr/include/x86_64-linux-gnu/qt5/QGSettings

include(interface.pri)
include(widgets.pri)

DEFINES += QT_DEPRECATED_WARNINGS

HEADERS += libukmge_global.h

unix {
    target.path = $$[QT_INSTALL_LIBS]

    widgets.path  = /usr/include/ukmge/widgets
    widgets.files = widgets/SwitchButton/*.h \
                    widgets/FlowLayout/*.h   \
                    widgets/SettingWidget/*.h \
                    *.h

    interfaces.path  = /usr/include/ukmge/interface
    interfaces.files = interface/*.h

    INSTALLS += widgets interfaces
}
!isEmpty(target.path): INSTALLS += target
