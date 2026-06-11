if(WIN32)
    list(APPEND FUNKIN_WIN32_RESOURCES ${FUNKIN_WIN32_RESOURCE})
endif()

set(FUNKIN_3RD_PARTY_DIR "${CMAKE_SOURCE_DIR}/funkin/third_party")

file(GLOB APP_SOURCES CONFIGURE_DEPENDS 
    "src/app/*.cpp"
    "src/app/project/*.cpp"
)

file(GLOB ASSET_SOURCES CONFIGURE_DEPENDS 
    "src/assets/*.cpp"
    "src/assets/loaders/*.cpp"
)

file(GLOB INPUT_SOURCES CONFIGURE_DEPENDS 
    "src/input/*.cpp"
    "src/platform/win32/*.cpp"
)

file(GLOB IMGUI_SOURCES CONFIGURE_DEPENDS "src/imgui/*.cpp")
file(GLOB FILESYSTEM_SOURCES CONFIGURE_DEPENDS "src/filesystem/*.cpp")
file(GLOB CACHE_SOURCES CONFIGURE_DEPENDS "src/cache/*.cpp")
file(GLOB RENDERER_SOURCES CONFIGURE_DEPENDS 
    "src/renderer/sprite/*.cpp"
    "src/renderer/camera/*.cpp"
)

file(GLOB SHADER_SOURCES CONFIGURE_DEPENDS 
    "src/shaders/*.cpp"
    "src/shaders/sprites/*.cpp"
)

file(GLOB SHARED_SOURCES CONFIGURE_DEPENDS "src/shared/*.cpp")

file(GLOB UI_SOURCES CONFIGURE_DEPENDS 
    "src/ui/*.cpp"
    "src/ui/project/*.cpp"
    "src/ui/editor/*.cpp"
)

file(GLOB QT_SOURCES CONFIGURE_DEPENDS "qt/*.qrc")

set(THIRD_PARTY_SOURCES
    ${FUNKIN_3RD_PARTY_DIR}/imgui/imgui.cpp
    ${FUNKIN_3RD_PARTY_DIR}/imgui/imgui_draw.cpp
    ${FUNKIN_3RD_PARTY_DIR}/imgui/imgui_tables.cpp
    ${FUNKIN_3RD_PARTY_DIR}/imgui/imgui_widgets.cpp
    ${FUNKIN_3RD_PARTY_DIR}/imgui/imgui_demo.cpp
    ${FUNKIN_3RD_PARTY_DIR}/imgui/backends/imgui_impl_win32.cpp
    ${FUNKIN_3RD_PARTY_DIR}/lz4/lib/lz4.c
)

add_executable(Funkin
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
    ${UI_SOURCES}
    ${QT_SOURCES}
    ${FUNKIN_WIN32_RESOURCES}
)

target_precompile_headers(Funkin PRIVATE
    "$<$<COMPILE_LANGUAGE:CXX>:${CMAKE_CURRENT_SOURCE_DIR}/src/shared/pch.hpp>"
)

target_include_directories(Funkin PRIVATE
    src
    ${FUNKIN_3RD_PARTY_DIR}/imgui
    ${FUNKIN_3RD_PARTY_DIR}/json/include
    ${FUNKIN_3RD_PARTY_DIR}/bgfx/bgfx/examples/common
    ${FUNKIN_3RD_PARTY_DIR}/stb
    ${FUNKIN_3RD_PARTY_DIR}/miniaudio
    ${FUNKIN_3RD_PARTY_DIR}/xxhash
    ${FUNKIN_3RD_PARTY_DIR}/lz4/lib
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
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Quick
    Qt6::Qml
    Qt6::Svg
    QWindowKit::Widgets
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

if(WIN32)
    get_target_property(_qmake_executable Qt6::qmake IMPORTED_LOCATION)
    get_filename_component(_qt_bin_dir "${_qmake_executable}" DIRECTORY)
    find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${_qt_bin_dir}")
endif()

set_target_properties(Funkin PROPERTIES 
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/export/$<CONFIG>"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/export/$<CONFIG>"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/export/$<CONFIG>"
)

message(STATUS "Qt version: ${Qt6_VERSION}")

add_custom_command(TARGET Funkin POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_CURRENT_SOURCE_DIR}/tools/shaderc.exe
        $<TARGET_FILE_DIR:Funkin>/bin/shaderc.exe

    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${FUNKIN_3RD_PARTY_DIR}/bgfx/bgfx/src
        $<TARGET_FILE_DIR:Funkin>/bin/shaderc_include
        
    ## qt legal!!!
    COMMAND "${WINDEPLOYQT_EXECUTABLE}" "$<TARGET_FILE:Funkin>"
        --dir "$<TARGET_FILE_DIR:Funkin>"
        --plugindir "$<TARGET_FILE_DIR:Funkin>/bin/qt/plugins"
        --no-compiler-runtime --no-translations --no-opengl-sw

    COMMAND ${CMAKE_COMMAND} -E echo "[Paths]"          > $<TARGET_FILE_DIR:Funkin>/qt.conf
    COMMAND ${CMAKE_COMMAND} -E echo "Prefix = bin/qt" >> $<TARGET_FILE_DIR:Funkin>/qt.conf
)