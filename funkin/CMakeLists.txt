if(WIN32)
    list(APPEND FUNKIN_WIN32_RESOURCES ${FUNKIN_WIN32_RESOURCE})
endif()

add_executable(Funkin
    src/app/main.cpp
    src/input/input.cpp
    src/platform/win32/win32_input.cpp
    src/imgui/imgui_impl_sdl3.cpp
    src/imgui/imgui_impl_bgfx.cpp
    src/app/debug.cpp
    src/app/projects/project.cpp
    src/app/projects/launcher.cpp
    ${FUNKIN_WIN32_RESOURCES}
    src/app/registry.cpp
    src/filesystem/filesystem.cpp
    src/assets/assets.cpp
    src/assets/loaders/texture_loader.cpp
    src/assets/loaders/audio_loader.cpp
    src/assets/loaders/json_loader.cpp
    src/assets/loaders/font_loader.cpp
)

set(BGFX_DIR ${CMAKE_SOURCE_DIR}/funkin/third_party/bgfx)
set(BGFX_IMGUI_DIR ${BGFX_DIR}/bgfx/examples/common)
set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/funkin/third_party/imgui)
set(JSON_DIR ${CMAKE_SOURCE_DIR}/funkin/third_party/json/include)
set(STB_DIR ${CMAKE_SOURCE_DIR}/funkin/third_party/stb)
set(MINIAUDIO_DIR ${CMAKE_SOURCE_DIR}/funkin/third_party/miniaudio)

target_sources(Funkin PRIVATE
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
)

target_precompile_headers(Funkin PRIVATE
    src/shared/pch.hpp
)

target_include_directories(Funkin PRIVATE
    src
    ${IMGUI_DIR}
    ${JSON_DIR}
    ${BGFX_IMGUI_DIR}
    ${STB_DIR}
    ${MINIAUDIO_DIR}
    ${BGFX_DIR}/bgfx/3rdparty
    ${BGFX_DIR}/bgfx/include
    ${BGFX_DIR}/bx/include
    ${BGFX_DIR}/bimg/include
)

target_link_libraries(Funkin PRIVATE 
    SDL3::SDL3-static 
    bgfx 
    bx 
    bimg
)

if(WIN32)
    target_link_libraries(Funkin PRIVATE
        d3d11
        dxgi
        d3dcompiler
        dxguid
        shlwapi
        ole32
        uuid
    )
    target_compile_definitions(Funkin PRIVATE
        NOMINMAX
        WIN32_LEAN_AND_MEAN
        _WIN32_WINNT=0x0601
    )
endif()

if(UNIX AND NOT APPLE)
    target_link_libraries(Funkin PRIVATE
        GL
        X11
        pthread
        dl
    )
endif()

if(FUNKIN_DEBUG)
    target_compile_definitions(Funkin PRIVATE FUNKIN_DEBUG)
endif()

add_custom_command(TARGET Funkin POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/assets
    $<TARGET_FILE_DIR:Funkin>/assets
)