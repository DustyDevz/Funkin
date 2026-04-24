file(GLOB_RECURSE Game_SOURCES CONFIGURE_DEPENDS "*.hpp" "*.cpp")

add_library(Game STATIC ${Game_SOURCES})
target_include_directories(Game PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(Game PRIVATE d3d12 dxgi)