cmake_minimum_required(VERSION 3.10)

project(linemt5)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include/)

add_compile_options(-fpermissive)

add_library(linemt5 SHARED
    main.cpp
    audio.cpp
    bana.cpp
    camera.cpp
    compress.cpp
    ffb.cpp
    gl.cpp
    hasp.cpp
    input.cpp
    mq.cpp
    net.cpp
    nuinput.cpp
    nvpm.cpp
    touch.cpp
    window.cpp

    shared/config.cpp
    shared/input.cpp
    shared/line.cpp
)

target_link_libraries(linemt5
    ${CMAKE_SOURCE_DIR}/lib/libSDL2.dll.a
    z
    opengl32
    gdi32
)

set_target_properties(linemt5 PROPERTIES
    PREFIX "msys-"
    OUTPUT_NAME "linemt5"
)
