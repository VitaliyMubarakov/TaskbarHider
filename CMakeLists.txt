cmake_minimum_required(VERSION 3.8)
project(TaskbarHider)

# Устанавливаем стандарт C++
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Настройка MSVC debug info
if(MSVC)
    set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "ProgramDatabase")
endif()

# Указываем исходники
set(SOURCES
    "TaskbarHider.cpp"
    "src/taskbarManager.cpp"
)

# Заголовки (чтобы IDE их видела)
set(HEADERS
    "src/taskbarManager.h"
    "libs/IniParser.h"
)

# Создаём исполняемый файл
add_executable(TaskbarHider WIN32 ${SOURCES} ${HEADERS})

# Копирование BAT файла после сборки
add_custom_command(TARGET TaskbarHider POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        "${CMAKE_SOURCE_DIR}/AddStartup.bat"
        $<TARGET_FILE_DIR:TaskbarHider>/AddStartup.bat
)
