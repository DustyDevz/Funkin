if(WIN32)
    list(APPEND FUNKIN_WIN32_RESOURCES ${FUNKIN_WIN32_RESOURCE})
endif()

add_executable(Funkin
    src/shaders/shader_uniform.cpp)

set(FUNKIN_3RD_PARTY_DIR "${CMAKE_SOURCE_DIR}/funkin/third_party")

set(APP_SOURCES
    src/app/main.cpp
    src/app/debug.cpp
    src/app/registry.cpp
    src/app/projects/project.cpp
    src/app/projects/launcher.cpp
)

set(INPUT_SOURCES
    src/input/input.cpp
    src/platform/win32/win32_input.cpp
    ${FUNKIN_WIN32_RESOURCES}
)

set(IMGUI_SOURCES
    src/imgui/imgui_impl_sdl3.cpp
    src/imgui/imgui_impl_bgfx.cpp
)

set(FILESYSTEM_SOURCES
    src/filesystem/filesystem.cpp
)

set(ASSET_SOURCES
    src/assets/assets.cpp
    src/assets/loaders/texture_loader.cpp
    src/assets/loaders/audio_loader.cpp
    src/assets/loaders/json_loader.cpp
    src/assets/loaders/font_loader.cpp
)

set(CACHE_SOURCES
    src/cache/cache.cpp
)

set(RENDERER_SOURCES
    src/renderer/sprite/sprite_batch.cpp
)

set(SHADER_SOURCES
    src/shaders/shader_compiler.cpp
    src/shaders/shader_converter.cpp
    src/shaders/shader_cache.cpp
    src/shaders/shader_job_queue.cpp
    src/shaders/shader_program.cpp
    src/shaders/sprites/sprite_shader.cpp
)

set(SHARED_SOURCES
    src/shared/utils.cpp
    src/shared/xxhash_impl.cpp
)

set(THIRD_PARTY_SOURCES
    ${FUNKIN_3RD_PARTY_DIR}/imgui/imgui.cpp
    ${FUNKIN_3RD_PARTY_DIR}/imgui/imgui_draw.cpp
    ${FUNKIN_3RD_PARTY_DIR}/imgui/imgui_tables.cpp
    ${FUNKIN_3RD_PARTY_DIR}/imgui/imgui_widgets.cpp
    ${FUNKIN_3RD_PARTY_DIR}/imgui/imgui_demo.cpp
)

target_sources(Funkin PRIVATE
    ${APP_SOURCES}
    ${INPUT_SOURCES}
    ${IMGUI_SOURCES}
    ${CACHE_SOURCES}
    ${FILESYSTEM_SOURCES}
    ${ASSET_SOURCES}
    ${RENDERER_SOURCES}
    ${SHADER_SOURCES}
    ${THIRD_PARTY_SOURCES}
    ${SHARED_SOURCES}
)

target_precompile_headers(Funkin PRIVATE
    src/shared/pch.hpp
)

target_include_directories(Funkin PRIVATE
    src
    ${FUNKIN_3RD_PARTY_DIR}/imgui
    ${FUNKIN_3RD_PARTY_DIR}/json/include
    ${FUNKIN_3RD_PARTY_DIR}/bgfx/bgfx/examples/common
    ${FUNKIN_3RD_PARTY_DIR}/stb
    ${FUNKIN_3RD_PARTY_DIR}/miniaudio
    ${FUNKIN_3RD_PARTY_DIR}/xxhash
    ${FUNKIN_3RD_PARTY_DIR}/bgfx/bgfx/3rdparty
    ${FUNKIN_3RD_PARTY_DIR}/bgfx/bgfx/include
    ${FUNKIN_3RD_PARTY_DIR}/bgfx/bx/include
    ${FUNKIN_3RD_PARTY_DIR}/bgfx/bimg/include
    ${FUNKIN_3RD_PARTY_DIR}/bgfx/bimg/3rdparty
)

target_link_libraries(Funkin PRIVATE 
    SDL3::SDL3-static 
    bgfx 
    bx 
    bimg
    bimg_encode
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

    COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_CURRENT_SOURCE_DIR}/tools/shaderc.exe
        $<TARGET_FILE_DIR:Funkin>/bin/shaderc.exe

    COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_CURRENT_SOURCE_DIR}/tools/varying.def.sc
        $<TARGET_FILE_DIR:Funkin>/bin/varying.def.sc

    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${FUNKIN_3RD_PARTY_DIR}/bgfx/bgfx/src
        $<TARGET_FILE_DIR:Funkin>/bin/shaderc_include
)