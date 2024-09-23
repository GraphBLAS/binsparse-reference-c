#-------------------------------------------------------------------------------
# binsparse-reference-c/Makefile
#-------------------------------------------------------------------------------

# GraphBLAS/binpsparse, TODO (c) 2024
# SPDX-License-Identifier: BSD-3-clause

#-------------------------------------------------------------------------------

# simple Makefile for binsparse, relies on cmake to do the actual build.  Use
# the CMAKE_OPTIONS argument to this Makefile to pass options to cmake.

default:
	( cd build && cmake $(CMAKE_OPTIONS) .. && cmake --build . --config Release )

# compile with -g 
debug:
	( cd build && cmake -DCMAKE_BUILD_TYPE=Debug $(CMAKE_OPTIONS) .. && cmake --build . --config Debug )

# run the tests
tests: default
	ctest --test-dir ./build/test/bash

# run the tests in verbose mode
vtests: default
	ctest -VV --test-dir ./build/test/bash

# installs binsparse to the install location defined by cmake, usually
# /usr/local/lib and /usr/local/include
install:
	( cd build && cmake --install . )

# remove any installed libraries and #include files
uninstall:
	- xargs rm < build/install_manifest.txt

clean: distclean

purge: distclean

# remove all files not in the distribution
distclean:
	- rm -rf build/*

