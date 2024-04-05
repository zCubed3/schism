cmake_minimum_required(VERSION 3.27)
project(schism)

set(CMAKE_CXX_STANDARD 17)

# ===============
#   Global Dirs
# ===============
set(SCHISM_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})

# ===============
#   Output Dirs
# ===============
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${SCHISM_ROOT_DIR}/bin/${CMAKE_CXX_COMPILER_ID}-${CMAKE_SYSTEM_PROCESSOR}-${CMAKE_BUILD_TYPE}/")
set(CMAKE_PDB_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY})

# ===========
#   Options
# ===========
option(SCHISM_BUILD_GUI "Builds the GUI" ON)

# ================
#   Dependencies
# ================
include(FetchContent)

if (SCHISM_BUILD_GUI)

    FetchContent_Declare(
            ImGui
            GIT_REPOSITORY https://github.com/ocornut/imgui.git
            GIT_TAG        085781f5ca5372d5fc804d7e44b5bf27a8994af7
    )

    FetchContent_Declare(
            sdl2
            GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
            GIT_TAG        f461d91cd265d7b9a44b4d472b1df0c0ad2855a0
    )

    FetchContent_MakeAvailable(imgui sdl2)

endif()

# ============
#   Projects
# ============
add_subdirectory(schism)

if (SCHISM_BUILD_GUI)
    add_subdirectory(schism_gui)
endif()