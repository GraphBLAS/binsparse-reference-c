# SPDX-FileCopyrightText: 2024 Binsparse Developers
#
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.20)

set ( BINSPARSE_DATE "FIXME, 2026" )
set ( BINSPARSE_VERSION_MAJOR 1 )
set ( BINSPARSE_VERSION_MINOR 0 )
set ( BINSPARSE_VERSION_PATCH 0 )

project(binsparse)

set ( PROJECT_VERSION
    "${BINSPARSE_VERSION_MAJOR}.${BINSPARSE_VERSION_MINOR}.${BINSPARSE_VERSION_PATCH}")

cmake_policy(SET CMP0079 NEW)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 20)

# FIXME: -march=native is not portable
# set(CMAKE_C_FLAGS "-O3 -march=native")
# set(CMAKE_C_FLAGS "-g ")
# set(CMAKE_CXX_FLAGS "-g ")

if ( NOT CMAKE_BUILD_TYPE )
    set ( CMAKE_BUILD_TYPE Release )
endif ( )

if ( ${CMAKE_BUILD_TYPE} STREQUAL "Debug" )
    set ( CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} ${CMAKE_C_FLAGS_DEBUG}" )
    set ( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_DEBUG}" )
else ( )
    set ( CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} ${CMAKE_C_FLAGS_RELEASE}" )
    set ( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_RELEASE}" )
endif ( )
message ( STATUS "Build type: ${CMAKE_BUILD_TYPE} ")
message ( STATUS "C   flags:  ${CMAKE_C_FLAGS}" )
message ( STATUS "C++ flags:  ${CMAKE_CXX_FLAGS}" )

option(ENABLE_SANITIZERS "Enable Clang sanitizers" OFF)

include(GNUInstallDirs)

add_library(binsparse_static STATIC)
add_library(binsparse_dynamic SHARED)

set_target_properties ( binsparse_static PROPERTIES
    OUTPUT_NAME binsparse
    VERSION "${BINSPARSE_VERSION_MAJOR}.${BINSPARSE_VERSION_MINOR}.${BINSPARSE_VERSION_PATCH}"
    SOVERSION ${BINSPARSE_VERSION_MAJOR}
    WINDOWS_EXPORT_ALL_SYMBOLS ON
    )

set_target_properties ( binsparse_dynamic PROPERTIES
    OUTPUT_NAME binsparse
    VERSION "${BINSPARSE_VERSION_MAJOR}.${BINSPARSE_VERSION_MINOR}.${BINSPARSE_VERSION_PATCH}"
    SOVERSION ${BINSPARSE_VERSION_MAJOR}
    WINDOWS_EXPORT_ALL_SYMBOLS ON
    )

add_subdirectory(include)
add_subdirectory(src)

# NOTE: For now, both HDF5 and cJSON are `PUBLIC`, meaning that anything that
# depends on `binsparse` will also link/include HDF5 and cJSON. We can change
# these to `PRIVATE` to use them only when building binsparse.

find_package(HDF5 REQUIRED COMPONENTS C)
target_link_libraries(binsparse_static PUBLIC ${HDF5_C_LIBRARIES})
target_link_libraries(binsparse_dynamic PUBLIC ${HDF5_C_LIBRARIES})
message ( STATUS "HD5F include: ${HDF5_INCLUDE_DIRS}" )

include(FetchContent)

# Force cJSON to build as static library for MEX compatibility
set(BUILD_SHARED_LIBS_BACKUP ${BUILD_SHARED_LIBS})
set(BUILD_SHARED_LIBS OFF CACHE INTERNAL "Build cJSON as static library")
set(ENABLE_CJSON_TEST OFF CACHE INTERNAL "Disable cJSON tests")

FetchContent_Declare(
  cJSON
  # GIT_REPOSITORY https://github.com/DaveGamble/cJSON.git
  GIT_REPOSITORY https://github.com/p1k0chu/cJSON.git
  GIT_TAG 887642c0a93bd8a6616bf90daacac0ea7d4b095e
)
FetchContent_MakeAvailable(cJSON)

# Restore original BUILD_SHARED_LIBS setting
set(BUILD_SHARED_LIBS ${BUILD_SHARED_LIBS_BACKUP})

configure_file(${cJSON_SOURCE_DIR}/cJSON.h ${CMAKE_BINARY_DIR}/include/cJSON/cJSON.h)
target_link_libraries(binsparse_static PRIVATE cjson)
target_link_libraries(binsparse_dynamic PRIVATE cjson)

# Set up include directories properly for both build and install
target_include_directories(binsparse_static
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
        ${HDF5_INCLUDE_DIRS})

target_include_directories(binsparse_dynamic
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
        ${HDF5_INCLUDE_DIRS})

# Installation rules - these are always needed when the library is built
install(TARGETS binsparse_static
        EXPORT binsparse-targets_static
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(TARGETS binsparse_dynamic
        EXPORT binsparse-targets_dynamic
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# Install headers
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/binsparse
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        FILES_MATCHING PATTERN "*.h"
                       PATTERN "*.hpp")

# Export targets
install(EXPORT binsparse-targets_static
        FILE binsparse-targets_static.cmake
        NAMESPACE binsparse::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/binsparse)
install(EXPORT binsparse-targets_dynamic
        FILE binsparse-targets_dynamic.cmake
        NAMESPACE binsparse::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/binsparse)

# Create and install package config files
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/binsparse-config-version.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/binsparse-config.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/binsparse-config.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/binsparse)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/binsparse-config.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/binsparse-config-version.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/binsparse)

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
  enable_testing()

  if (ENABLE_SANITIZERS)
      set(SANITIZER_FLAGS "-fsanitize=address,undefined")
      target_compile_options(binsparse_static INTERFACE ${SANITIZER_FLAGS} -g -O1 -fno-omit-frame-pointer)
      target_link_options(binsparse_static INTERFACE ${SANITIZER_FLAGS})
      target_compile_options(binsparse_dynamic INTERFACE ${SANITIZER_FLAGS} -g -O1 -fno-omit-frame-pointer)
      target_link_options(binsparse_dynamic INTERFACE ${SANITIZER_FLAGS})
  endif()

  add_subdirectory(examples)
  add_subdirectory(test)
endif()
