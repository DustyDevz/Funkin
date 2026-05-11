file(GLOB_RECURSE Funkin_SOURCES CONFIGURE_DEPENDS "src/*.cpp")

set(IMGUI_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third_party/imgui")
set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
)

if(WIN32)
    list(APPEND IMGUI_SOURCES 
        ${IMGUI_DIR}/backends/imgui_impl_win32.cpp
        ${IMGUI_DIR}/backends/imgui_impl_dx12.cpp
    )
    list(FILTER Funkin_SOURCES EXCLUDE REGEX ".*x11_window\\.cpp$")
    set(PLATFORM_LIBS d3d12 dxgi dwmapi)
    set(EXE_TYPE WIN32)
elseif(UNIX AND NOT APPLE)
    list(FILTER Funkin_SOURCES EXCLUDE REGEX ".*win32_window\\.cpp$|.*dx12.*\\.cpp$")
    set(PLATFORM_LIBS X11)
endif()

add_executable(Funkin ${EXE_TYPE} ${Funkin_SOURCES} ${IMGUI_SOURCES})

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(Funkin PRIVATE
        PACK_PATH="${CMAKE_SOURCE_DIR}/export/debug/assets.funkin"
        SOURCE_PATH="${CMAKE_CURRENT_SOURCE_DIR}/assets/"
    )
else()
    target_compile_definitions(Funkin PRIVATE
        PACK_PATH="assets.funkin"
        SOURCE_PATH=""
    )
endif()

find_package(Vulkan REQUIRED)

target_include_directories(Funkin
    BEFORE
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/directx/include
)

target_include_directories(Funkin
    PUBLIC src
    PRIVATE
        third_party/zstd/lib
        third_party/imgui
        third_party/imgui/backends
        third_party/stb
)

if(WIN32)
    target_link_libraries(Funkin PRIVATE XInput winmm ${PLATFORM_LIBS})
endif()

target_link_libraries(Funkin PRIVATE
    Vulkan::Vulkan
    libzstd_static
)

target_precompile_headers(Funkin PRIVATE src/pch.hpp)

target_compile_definitions(Funkin PRIVATE
    $<$<CONFIG:Debug>:FUNKIN_DEBUG>
    $<$<CONFIG:Release>:FUNKIN_RELEASE>
    $<$<BOOL:${FORCE_PRINT_ON_RELEASE}>:FORCE_PRINT_ON_RELEASE>
    D3D12_IGNORE_SDK_VERSION
    IMGUI_DEFINE_MATH_OPERATORS
)