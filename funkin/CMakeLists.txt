file(GLOB_RECURSE Funkin_SOURCES CONFIGURE_DEPENDS "*.hpp" "*.cpp")

add_library(Funkin STATIC ${Funkin_SOURCES})
target_include_directories(Funkin PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(Funkin PRIVATE d3d12 dxgi)