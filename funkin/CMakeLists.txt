file(GLOB_RECURSE Funkin_SOURCES CONFIGURE_DEPENDS "src/*.cpp")

if(WIN32)
    list(FILTER Funkin_SOURCES EXCLUDE REGEX ".*x11_window\\.cpp$")
    set(PLATFORM_LIBS d3d12 dxgi dwmapi)
    set(EXE_TYPE WIN32)
elseif(UNIX AND NOT APPLE)
    list(FILTER Funkin_SOURCES EXCLUDE REGEX ".*win32_window\\.cpp$|.*dx12.*\\.cpp$")
    set(PLATFORM_LIBS X11)
endif()

add_executable(Funkin ${EXE_TYPE} ${Funkin_SOURCES})

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
)