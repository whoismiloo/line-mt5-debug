cmake_minimum_required(VERSION 3.10)

project(linemt5cmake)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include/)

add_library(linemt5debug SHARED main.cpp
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

target_link_libraries(linemt5debug ${CMAKE_SOURCE_DIR}/lib/libSDL2.dll.a z opengl32 gdi32)