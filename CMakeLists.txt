cmake_minimum_required(VERSION 3.20)
project(binsparse-rc)

set(CMAKE_C_STANDARD 17)

set(CMAKE_C_FLAGS "-O3 -march=native")

add_subdirectory(include)

find_package(HDF5 REQUIRED COMPONENTS C)
target_link_libraries(binsparse-rc INTERFACE ${HDF5_C_LIBRARIES})
target_include_directories(binsparse-rc INTERFACE . ${HDF5_INCLUDE_DIRS})

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
  add_subdirectory(examples)
endif()