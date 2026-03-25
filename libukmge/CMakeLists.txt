# Modern CMakeLists.txt for libukmge (biblioteca Qt)

set(CMAKE_AUTOMOC ON)

add_library(ukmge
    widgets/FlowLayout/flowlayout.cpp
    widgets/SettingWidget/ukccframe.cpp
    widgets/SwitchButton/switchbutton.cpp
    widgets/CloseButton/closebutton.cpp
    libukmge_global.h
    interface/interface.h
    widgets/FlowLayout/flowlayout.h
    widgets/SettingWidget/ukccframe.h
    widgets/SwitchButton/switchbutton.h
    widgets/CloseButton/closebutton.h
)

target_include_directories(ukmge PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/interface
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/FlowLayout
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/SettingWidget
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/SwitchButton
)

target_link_libraries(ukmge
    PUBLIC
        Qt5::Core
        Qt5::Widgets
        Qt5::DBus
)

# PkgConfig-based gio dependencies for Unix systems
find_package(PkgConfig REQUIRED)
pkg_check_modules(GIO2 REQUIRED gio-2.0 gio-unix-2.0)
pkg_check_modules(GSETTINGS_QT REQUIRED gsettings-qt)
target_link_libraries(ukmge PUBLIC ${GIO2_LIBRARIES} ${GSETTINGS_QT_LIBRARIES})
target_include_directories(ukmge PUBLIC ${GIO2_INCLUDE_DIRS} ${GSETTINGS_QT_INCLUDE_DIRS})

# Export macro for shared library
set_target_properties(ukmge PROPERTIES
    DEFINE_SYMBOL LIBUKMGE_LIBRARY
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON
)

install(TARGETS ukmge
        DESTINATION lib)

install(FILES
    libukmge_global.h
    interface/interface.h
    widgets/FlowLayout/flowlayout.h
    widgets/SettingWidget/ukccframe.h
    widgets/SwitchButton/switchbutton.h
    DESTINATION include/ukmge)
