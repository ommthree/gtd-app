cmake_minimum_required(VERSION 3.15)
project(GTDApp)

set(CMAKE_CXX_STANDARD 17)

# --- Core + UI Components ---
file(GLOB_RECURSE CORE_SRC
    "src/core/*.cpp"
    "src/core/*.h"
    "src/ui/*.cpp"
    "src/ui/*.h"
)

# --- Platform-Specific (Windows) ---
if(WIN32)
    set(PLATFORM_SRC
        src/platform/windows/gui_win32.cpp
        src/platform/windows/gui_win32.h
    )
    add_definitions(-DPLATFORM_WINDOWS)
endif()

# --- ImGui ---
file(GLOB IMGUI_SRC
    "third_party/imgui/*.cpp"
    "third_party/imgui/backends/imgui_impl_win32.cpp"
    "third_party/imgui/backends/imgui_impl_dx11.cpp"
)

# --- Main Entry Point ---
set(MAIN_SRC
    src/main.cpp
)

# --- Add SQLite3 ---
add_library(sqlite3 STATIC third_party/sqlite/sqlite3.c "src/ui/canvas_view.h")
target_include_directories(sqlite3 PUBLIC third_party/sqlite)

# --- Define Executable ---
add_executable(GTDApp
    ${MAIN_SRC}
    ${CORE_SRC}
    ${PLATFORM_SRC}
    ${IMGUI_SRC}
 "src/ui/canvas_view.h")

# --- Include Directories ---
target_include_directories(GTDApp PRIVATE
    src
    src/core
    src/ui
    src/platform/windows
    third_party/imgui
    third_party/imgui/backends
    third_party/mysql-connector-c/include
    third_party/json
)

# --- Link Libraries ---
target_link_libraries(GTDApp PRIVATE
    sqlite3
    "${CMAKE_SOURCE_DIR}/third_party/mysql-connector-c/lib/libmysql.lib"
)

# --- Windows System Libraries ---
if(WIN32)
    target_link_libraries(GTDApp PRIVATE d3d11 dxgi dxguid)
endif()

# --- Post-build: Copy DLLs for MySQL connector ---
add_custom_command(TARGET GTDApp POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_SOURCE_DIR}/third_party/mysql-connector-c/lib/libmysql.dll"
    $<TARGET_FILE_DIR:GTDApp>
)

add_custom_command(TARGET GTDApp POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_SOURCE_DIR}/third_party/mysql-connector-cpp/lib64/libcrypto-3-x64.dll"
    $<TARGET_FILE_DIR:GTDApp>
)

add_custom_command(TARGET GTDApp POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_SOURCE_DIR}/third_party/mysql-connector-cpp/lib64/libssl-3-x64.dll"
    $<TARGET_FILE_DIR:GTDApp>
)