add_executable(FNFCPP
    "src/core/main.cpp"
)

target_include_directories(FNFCPP PRIVATE
    src
    ${CMAKE_SOURCE_DIR}/third_party/bgfx/include
    ${CMAKE_SOURCE_DIR}/third_party/bx/include
    ${CMAKE_SOURCE_DIR}/third_party/bimg/include
)

target_link_libraries(FNFCPP PRIVATE bgfx bx bimg)

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