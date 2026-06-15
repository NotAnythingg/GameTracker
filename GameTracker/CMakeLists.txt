cmake_minimum_required(VERSION 3.15)
project(GameTracker)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SDL2_DIR "C:/SDL2")
set(SDL2_IMAGE_DIR "C:/SDL2_image")
set(IMGUI_DIR "${CMAKE_SOURCE_DIR}/imgui/imgui-master")

add_executable(GameTracker WIN32
    main.cpp
    database.cpp
    sqlite3.c
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_sdl2.cpp
    ${IMGUI_DIR}/backends/imgui_impl_sdlrenderer2.cpp
)

target_include_directories(GameTracker PRIVATE 
    ${SDL2_DIR}/include
    ${SDL2_IMAGE_DIR}/include
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
)

target_link_directories(GameTracker PRIVATE 
    ${SDL2_DIR}/lib/x64
    ${SDL2_IMAGE_DIR}/lib/x64
)

target_link_libraries(GameTracker PRIVATE 
    SDL2.lib
    SDL2main.lib
    SDL2_image.lib
)

add_custom_command(TARGET GameTracker POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${SDL2_DIR}/lib/x64/SDL2.dll
    ${SDL2_IMAGE_DIR}/lib/x64/SDL2_image.dll
    $<TARGET_FILE_DIR:GameTracker>
)

target_compile_definitions(GameTracker PRIVATE SQLITE_THREADSAFE=1)

if(MSVC)
    target_compile_options(GameTracker PRIVATE /W4)
endif()