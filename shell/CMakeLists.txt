# CMakeLists.txt for shell (executável principal)

# Habilita automoc/autouic/autorcc do Qt6
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
# set(CMAKE_AUTOUIC ON)  # AUTOUIC não necessário, UI é C++ only

set(SHELL_SOURCES
    ../backend/xdgpaths.cpp
    ../backend/desktopfilehandler.cpp
    ../backend/menuparser.cpp
    ../backend/menuwriter.cpp
    main.cpp
    mainwindow.cpp
    titlewidget.cpp
    leftsidebarwidget.cpp
    mstackwidget.cpp
    menutreewidget.cpp
    applicationeditorwidget.cpp
    editorwidget.cpp
    searchbar.cpp
)

set(SHELL_HEADERS
    ../backend/xdgpaths.h
    ../backend/desktopfilehandler.h
    ../backend/menuparser.h
    ../backend/menuwriter.h
    mainwindow.h
    titlewidget.h
    leftsidebarwidget.h
    mstackwidget.h
    menutreewidget.h
    applicationeditorwidget.h
    editorwidget.h
    searchbar.h
)


set(SHELL_RESOURCES
    res/resfile.qrc
)

add_executable(openkylinmenueditor
    ${SHELL_SOURCES}
    ${SHELL_HEADERS}
    ${SHELL_RESOURCES}
)

# Garante que o executável procure as libs em ../libukmge relativo à pasta bin
set_target_properties(openkylinmenueditor PROPERTIES
    BUILD_WITH_INSTALL_RPATH TRUE
    INSTALL_RPATH "$ORIGIN/../libukmge"
)

target_include_directories(openkylinmenueditor PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ../backend
)

target_link_libraries(openkylinmenueditor
    PRIVATE
        ukmge
        Qt5::Core
        Qt5::Gui
        Qt5::Widgets
        Qt5::Xml
        Qt5::DBus

)

# PkgConfig dependencies: gio, gio-unix, libmenu-cache
find_package(PkgConfig REQUIRED)
pkg_check_modules(GIO2 REQUIRED gio-2.0 gio-unix-2.0)
pkg_check_modules(MENU_CACHE REQUIRED libmenu-cache)
target_link_libraries(openkylinmenueditor PRIVATE ${GIO2_LIBRARIES} ${MENU_CACHE_LIBRARIES})
target_include_directories(openkylinmenueditor PRIVATE ${GIO2_INCLUDE_DIRS} ${MENU_CACHE_INCLUDE_DIRS})

# Define macro OPENKYLINMENUEDITOR
add_definitions(-DOPENKYLINMENUEDITOR)


install(TARGETS openkylinmenueditor DESTINATION bin)

install(FILES openkylinmenueditor.desktop DESTINATION share/applications)

# Instala arquivos .qm de tradução, se existirem
install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/ DESTINATION share/openkylinmenueditor/i18n
    FILES_MATCHING PATTERN "*.qm")

install(FILES ../data/openkylinmenueditor-config.json
    DESTINATION share/openkylinmenueditor/data)

# Instalar recursos?
install(FILES res/resfile.qrc DESTINATION share/openkylinmenueditor/res)

# Traduções Qt
# Removido qt_add_translations: comando exclusivo do Qt6. Traduções devem ser geradas manualmente com lrelease, se necessário.

