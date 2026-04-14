# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Release")
  file(REMOVE_RECURSE
  "CMakeFiles/colormenoisy_autogen.dir/AutogenUsed.txt"
  "CMakeFiles/colormenoisy_autogen.dir/ParseCache.txt"
  "colormenoisy_autogen"
  )
endif()
