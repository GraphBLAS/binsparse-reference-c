#-------------------------------------------------------------------------------
# binsparse-reference-c/Makefile
#-------------------------------------------------------------------------------

# SPDX-License-Identifier: TODO

#-------------------------------------------------------------------------------

# simple Makefile for binsparse-reference-c, relies on cmake to do the actual
# build.  Use the CMAKE_OPTIONS argument to this Makefile to pass options to
# cmake.  For example, to compile with 40 threads, use:
#
#       make JOBS=40

JOBS ?= 8

default: library

library:
	( cd build && cmake $(F) $(CMAKE_OPTIONS) .. && cmake --build . --config Release -j${JOBS} )

# install only in SuiteSparse/lib and SuiteSparse/include
local:
	( cd build && cmake $(F) $(CMAKE_OPTIONS) -USUITESPARSE_PKGFILEDIR -DSUITESPARSE_LOCAL_INSTALL=1 .. && cmake --build . --config Release -j${JOBS} )

# install only in /usr/local (default)
global:
	( cd build && cmake $(F) $(CMAKE_OPTIONS) -USUITESPARSE_PKGFILEDIR -DSUITESPARSE_LOCAL_INSTALL=0 .. && cmake --build . --config Release -j${JOBS} )

# compile with -g
debug:
	( cd build && cmake -DCMAKE_BUILD_TYPE=Debug $(F) $(CMAKE_OPTIONS) .. && cmake --build . --config Debug -j$(JOBS) )

# run the demos
demos: all
	FIXME

# just do 'make' in build; do not rerun the cmake script
remake:
	( cd build && cmake --build . -j$(JOBS) )

# just run cmake; do not compile
setup:
	( cd build && cmake $(F) $(CMAKE_OPTIONS) .. )

# build the static library
static:
	( cd build && cmake $(F) $(CMAKE_OPTIONS) -DBUILD_STATIC_LIBS=ON -DBUILD_SHARED_LIBS=OFF .. && cmake --build . --config Release -j$(JOBS) )

# installs to the install location defined by cmake, usually
# /usr/local/lib and /usr/local/include
install:
	( cd build && cmake --install . )

# create the doc
docs:
	( cd Doc && $(MAKE) )

# remove any installed libraries and #include files
uninstall:
	- xargs rm < build/install_manifest.txt

clean: distclean

purge: distclean

# remove all files not in the distribution
distclean:
	- rm -rf build/*
