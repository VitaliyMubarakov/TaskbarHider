# CMakeList.txt : CMake project for TaskbarHider, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("TaskbarHider")

set(SOURCES
    "TaskbarHider.cpp"
    "src/taskbarManager.cpp"
)

set(HEADERS
    "src/taskbarManager.h"
    "libs/IniParser.h"
)

# Add source to this project's executable.
add_executable(TaskbarHider ${SOURCES} ${HEADERS})

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET TaskbarHider PROPERTY CXX_STANDARD 20)
endif()

# TODO: Add tests and install targets if needed.
