cmake_minimum_required(VERSION 3.10)
project(openkylinmenueditor LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt5 REQUIRED COMPONENTS Widgets Core DBus Xml LinguistTools)

# Melhora navegação/autocomplete CLion
include_directories(
    ${PROJECT_SOURCE_DIR}/libukmge
    ${PROJECT_SOURCE_DIR}/libukmge/widgets/FlowLayout
    ${PROJECT_SOURCE_DIR}/libukmge/widgets/SettingWidget
    ${PROJECT_SOURCE_DIR}/libukmge/widgets/SwitchButton
    ${PROJECT_SOURCE_DIR}/libukmge/interface
    ${PROJECT_SOURCE_DIR}/backend
    ${PROJECT_SOURCE_DIR}/shell
)

add_subdirectory(libukmge)
add_subdirectory(shell)


