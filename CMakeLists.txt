cmake_minimum_required(VERSION 3.10)

project(renderer)
set (CMAKE_CXX_STANDARD 11)

add_executable(${PROJECT_NAME})

target_sources(${PROJECT_NAME} PRIVATE
  src/main.cpp
  src/Color.cpp
  src/Image.cpp
  src/Camera.cpp
  src/Sphere.cpp
  src/Scene.cpp
  src/constants.cpp
  src/Light.cpp
  src/Lambert.cpp
  src/Emission.cpp
  src/Shape.cpp
  src/Sampling.cpp
  src/TriangleMesh.cpp
  src/AABB.cpp
  src/BVH.cpp
  src/Microfacet.cpp
  src/LightPath.cpp
  src/transform.cpp
  )

find_package(OpenMP REQUIRED)

target_link_libraries(${PROJECT_NAME} PRIVATE OpenMP::OpenMP_CXX SDL2)
target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wno-unused-parameter)

target_include_directories(${PROJECT_NAME} PRIVATE src/extern)

