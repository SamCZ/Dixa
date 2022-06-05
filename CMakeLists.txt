cmake_minimum_required(VERSION 3.21)
project(DX11_PerfTests)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(lib)

file(GLOB_RECURSE PROJECT_SOURCE src/*.hpp src/*.cpp)
add_executable(DX11_PerfTests ${PROJECT_SOURCE})

target_link_libraries(DX11_PerfTests glfw d3d11 dxguid d3dcompiler)