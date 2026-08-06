#-------------------------------------------------------------------------------
# binsparse-reference-c/Makefile
#-------------------------------------------------------------------------------

# SPDX-FileCopyrightText: 2024 Binsparse Developers
# SPDX-License-Identifier: BSD-3-Clause

default: library

library:
	( cd build && cmake $(CMAKE_OPTIONS) .. && cmake --build . --config Release )

debug:
	( cd build && cmake $(CMAKE_OPTIONS) -DCMAKE_BUILD_TYPE=Debug .. ; cmake --build . --config Debug )

all: library

tests:
	( cd build ; make test )

install:
	( cd build && cmake --install . )

# remove any installed libraries and #include files
uninstall:
	- xargs rm < build/install_manifest.txt

# remove all files not in the distribution
clean: distclean

purge: distclean

distclean:
	- $(RM) -rf build/* Config/*.tmp

docs:

