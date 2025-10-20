cmake_minimum_required(VERSION 3.8)
project(TaskbarHider)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "ProgramDatabase")
endif()

set(SOURCES
    "TaskbarHider.cpp"
    "src/taskbarTray.cpp"
    "src/taskbarManager.cpp"
)

set(HEADERS
    "src/taskbarManager.h"
    "libs/IniParser.h"
    "src/taskbarTray.h")

add_executable(TaskbarHider WIN32 ${SOURCES} ${HEADERS})

add_definitions(-DUNICODE -D_UNICODE)

add_custom_command(TARGET TaskbarHider POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        "${CMAKE_SOURCE_DIR}/AddStartup.bat"
        $<TARGET_FILE_DIR:TaskbarHider>/AddStartup.bat
)
