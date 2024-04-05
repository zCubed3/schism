# ===========================
#   Schism GUI Build Target
# ===========================
file(GLOB_RECURSE SCHISM_GUI_SRC_FILES
    *.cpp
    *.hpp
)

list(APPEND SCHISM_GUI_SRC_FILES
    "${imgui_SOURCE_DIR}/backends/imgui_impl_sdl2.cpp"
    "${imgui_SOURCE_DIR}/backends/imgui_impl_sdlrenderer2.cpp"
    "${imgui_SOURCE_DIR}/misc/cpp/imgui_stdlib.cpp"
    "${imgui_SOURCE_DIR}/imgui_demo.cpp"
    "${imgui_SOURCE_DIR}/imgui_draw.cpp"
    "${imgui_SOURCE_DIR}/imgui_tables.cpp"
    "${imgui_SOURCE_DIR}/imgui_widgets.cpp"
    "${imgui_SOURCE_DIR}/imgui.cpp"
)

add_executable(SchismGUI ${SCHISM_GUI_SRC_FILES})

target_include_directories(SchismGUI PUBLIC
    ${SCHISM_ROOT_DIR}
    ${imgui_SOURCE_DIR}
    ${stb_SOURCE_DIR}
    SDL2
)

target_link_libraries(SchismGUI PUBLIC
    Schism
    SDL2
    SDL2main
)