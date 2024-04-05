# =======================
#   Schism Build Target
# =======================
file(GLOB_RECURSE SCHISM_SRC_FILES
    *.cpp
    *.hpp
)

add_library(Schism STATIC ${SCHISM_SRC_FILES})

target_include_directories(Schism PUBLIC
    ${SCHISM_ROOT_DIR}
)

add_custom_target(copy-asm ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${SCHISM_ROOT_DIR}/example_asm
    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}asm
)

add_dependencies(Schism copy-asm)