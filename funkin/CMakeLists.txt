add_executable(FNFCPP
    src/app/main.cpp
    src/input/input.cpp
    src/platform/win32/win32_input.cpp
    src/imgui/imgui_impl_sdl3.cpp
    src/imgui/imgui_impl_bgfx.cpp
    src/app/debug.cpp
)

set(BGFX_DIR ${CMAKE_SOURCE_DIR}/funkin/third_party/bgfx)
set(BGFX_IMGUI_DIR ${BGFX_DIR}/bgfx/examples/common)
set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/funkin/third_party/imgui)
set(JSON_DIR ${CMAKE_SOURCE_DIR}/funkin/third_party/json)

target_sources(FNFCPP PRIVATE
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
)

target_precompile_headers(FNFCPP PRIVATE
    src/shared/pch.hpp
)

target_include_directories(FNFCPP PRIVATE
    src
    ${IMGUI_DIR}
    ${JSON_DIR}
    ${BGFX_IMGUI_DIR}
    ${BGFX_DIR}/bgfx/3rdparty
    ${BGFX_DIR}/bgfx/include
    ${BGFX_DIR}/bx/include
    ${BGFX_DIR}/bimg/include
)

target_link_libraries(FNFCPP PRIVATE 
    SDL3::SDL3-static 
    bgfx 
    bx 
    bimg
)

if(WIN32)
    target_link_libraries(FNFCPP PRIVATE
        d3d11
        dxgi
        d3dcompiler
        dxguid
    )
    target_compile_definitions(FNFCPP PRIVATE
        NOMINMAX
        WIN32_LEAN_AND_MEAN
        _WIN32_WINNT=0x0601
    )
endif()

if(UNIX AND NOT APPLE)
    target_link_libraries(FNFCPP PRIVATE
        GL
        X11
        pthread
        dl
    )
endif()

if(FUNKIN_DEBUG)
    target_compile_definitions(FNFCPP PRIVATE FUNKIN_DEBUG)
endif()

add_custom_command(TARGET FNFCPP POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/assets
    $<TARGET_FILE_DIR:FNFCPP>/assets
)