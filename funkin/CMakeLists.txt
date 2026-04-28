file(GLOB_RECURSE Funkin_SOURCES CONFIGURE_DEPENDS
    "src/*.cpp"
)

if(WIN32)
    list(FILTER Funkin_SOURCES EXCLUDE REGEX ".*x11_window\\.cpp$")
elseif(UNIX AND NOT APPLE)
    list(FILTER Funkin_SOURCES EXCLUDE REGEX ".*win32_window\\.cpp$")
    list(FILTER Funkin_SOURCES EXCLUDE REGEX ".*dx12.*\\.cpp$")
endif()

if(WIN32)
    add_executable(Funkin WIN32 ${Funkin_SOURCES})
else()
    add_executable(Funkin ${Funkin_SOURCES})
endif()

find_package(Vulkan REQUIRED)

if(WIN32)
    target_link_libraries(Funkin PRIVATE d3d12 dxgi dwmapi Vulkan::Vulkan)
elseif(UNIX AND NOT APPLE)
    target_link_libraries(Funkin PRIVATE Vulkan::Vulkan X11)
endif()

target_include_directories(Funkin PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

### zstd
target_link_libraries(Funkin PRIVATE libzstd_static)
target_include_directories(Funkin PRIVATE
    ${CMAKE_SOURCE_DIR}/funkin/third_party/zstd/lib
)

target_compile_definitions(Funkin PRIVATE
    $<$<CONFIG:Debug>:FUNKIN_DEBUG>
    $<$<CONFIG:Release>:FUNKIN_RELEASE>
)