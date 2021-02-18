cmake_minimum_required(VERSION 3.10)

project(renderer)

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
  )
