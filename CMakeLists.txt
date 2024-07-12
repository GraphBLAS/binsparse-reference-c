cmake_minimum_required(VERSION 3.5)
project(binsparse-rc)

cmake_policy(SET CMP0079 NEW)

set(CMAKE_C_STANDARD 11)

set(CMAKE_C_FLAGS "-O3 -march=native")

add_subdirectory(include)

find_package(HDF5 REQUIRED COMPONENTS C)
target_link_libraries(binsparse-rc INTERFACE ${HDF5_C_LIBRARIES})
target_include_directories(binsparse-rc INTERFACE . ${HDF5_INCLUDE_DIRS})

include(FetchContent)
FetchContent_Declare(
  cJSON
  GIT_REPOSITORY https://github.com/DaveGamble/cJSON.git
  GIT_TAG v1.7.17
)
FetchContent_MakeAvailable(cJSON)

configure_file(${cJSON_SOURCE_DIR}/cJSON.h ${CMAKE_BINARY_DIR}/include/cJSON/cJSON.h)
target_include_directories(${PROJECT_NAME} INTERFACE ${CMAKE_BINARY_DIR}/include)
target_link_libraries(${PROJECT_NAME} INTERFACE cjson)

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
  add_subdirectory(examples)
  add_subdirectory(test)
endif()
