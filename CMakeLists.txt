cmake_minimum_required(VERSION 3.10)

project(renderer)
set (CMAKE_CXX_STANDARD 11)

# Use libANN
add_subdirectory(src/extern/ann_1.1.2/)

# use toml11
add_subdirectory(toml11)

# target renderer
add_executable(renderer)
target_include_directories(renderer PRIVATE ./src/extern/ann_1.1.2/include toml11 src/extern)
target_sources(renderer PRIVATE
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
  src/LightTree.cpp
  src/Transform.cpp
  src/LightPathExpression.cpp
  src/TOMLParser.cpp
  src/display.cpp
  )

find_package(OpenMP REQUIRED)

target_link_libraries(renderer PRIVATE OpenMP::OpenMP_CXX SDL2 toml11)
set(WARNING_OPTIONS -Wall -Wextra -Wno-unused-parameter)
target_compile_options(renderer PRIVATE ${WARNING_OPTIONS})


# target stylit
add_executable(stylit)

target_include_directories(stylit PRIVATE ./src/extern/ann_1.1.2/include src/extern)
target_sources(stylit PRIVATE
  src/stylit/main.cpp
  src/stylit/FeatureVector.cpp
  src/stylit/image_analogies.cpp
  src/stylit/stylit.cpp
  src/Color.cpp
  src/Image.cpp
  src/constants.cpp
  src/display.cpp
  )
target_link_libraries(stylit PRIVATE SDL2 ann)
target_compile_options(stylit PRIVATE ${WARNING_OPTIONS})

