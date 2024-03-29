# =======================
#   Schism Build Target
# =======================
file(GLOB_RECURSE SCHISM_SRC_FILES
    *.cpp
    *.hpp
)

add_executable(schism ${SCHISM_SRC_FILES})

target_include_directories(schism PUBLIC ${SCHISM_ROOT_DIR})