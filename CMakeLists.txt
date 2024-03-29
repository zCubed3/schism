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

# ================
#   Dependencies
# ================
include(FetchContent)

FetchContent_Declare(
        STB
        GIT_REPOSITORY https://github.com/nothings/stb.git
        GIT_TAG        ae721c50eaf761660b4f90cc590453cdb0c2acd0
)

FetchContent_MakeAvailable(STB)

# ============
#   Projects
# ============
add_subdirectory(schism)
