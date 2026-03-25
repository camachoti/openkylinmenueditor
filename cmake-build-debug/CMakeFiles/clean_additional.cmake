# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "libukmge/CMakeFiles/ukmge_autogen.dir/AutogenUsed.txt"
  "libukmge/CMakeFiles/ukmge_autogen.dir/ParseCache.txt"
  "libukmge/ukmge_autogen"
  "shell/CMakeFiles/openkylinmenueditor_autogen.dir/AutogenUsed.txt"
  "shell/CMakeFiles/openkylinmenueditor_autogen.dir/ParseCache.txt"
  "shell/openkylinmenueditor_autogen"
  )
endif()
