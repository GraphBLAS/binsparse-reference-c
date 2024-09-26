<!--
SPDX-FileCopyrightText: 2024 Binsparse Developers

SPDX-License-Identifier: BSD-3-Clause
-->

# Binsparse C Reference Implementation

This library is a reference implementation of the [binsparse Binary Sparse Format Specification](https://github.com/GraphBLAS/binsparse-specification) written using C.

Binsparse is a cross-platform, embeddable format for storing sparse matrices.  This library currently only uses HDF5 as the underlying binary container format.

## C Binsparse Interface

This library provides a C interface for reading and writing binsparse matrices.  The library uses runtime polymorphism to allow reading and writing matries of different types and formats.

```c
#include <binsparse/binsparse.h>

int main(int argc, char** argv) {
  bsp_matrix_t mat = bsp_read_matrix("chesapeake.bsp.hdf5");

  if (mat.format == BSP_COO) {
    float* values = mat.values.data;
    int* rowind = mat.indices_0.data;
    int* colind = mat.indices_1.data;

    for (size_t i = 0; i < mat.nnz; i++) {
      printf("%d, %d: %f\n", rowind[i], colind[i], values[i]);
    }
  } else {
    printf("Matrix format read was %s, format currently not handled.\n",
           bsp_get_matrix_format_string(mat.format));
  }

  return 0;
}
```

## Building

This library uses CMake.  It should be able to automatically download and link
all dependencies except for:

- A recent C compiler.
- libhdf5

HDF5 should be automatically detected, provided an installation is present on
the system.

```bash
bbrock@rjohns3-mobl2:~/src/binsparse-reference-c$ cmake -B build
-- The C compiler identification is AppleClang 15.0.0.15000309
-- The CXX compiler identification is AppleClang 15.0.0.15000309
-- Detecting C compiler ABI info
-- Detecting C compiler ABI info - done
-- Check for working C compiler: /Library/Developer/CommandLineTools/usr/bin/cc - skipped
-- Detecting C compile features
-- Detecting C compile features - done
-- Detecting CXX compiler ABI info
-- Detecting CXX compiler ABI info - done
-- Check for working CXX compiler: /Library/Developer/CommandLineTools/usr/bin/c++ - skipped
-- Detecting CXX compile features
-- Detecting CXX compile features - done
-- Found HDF5: /opt/homebrew/Cellar/hdf5/1.14.3/lib/libhdf5.dylib;/opt/homebrew/opt/libaec/lib/libsz.dylib;/Library/Developer/CommandLineTools/SDKs/MacOSX14.4.sdk/usr/lib/libz.tbd;/Library/Developer/CommandLineTools/SDKs/MacOSX14.4.sdk/usr/lib/libdl.tbd;/Library/Developer/CommandLineTools/SDKs/MacOSX14.4.sdk/usr/lib/libm.tbd (found version "1.14.3") found components: C
CMake Deprecation Warning at build/_deps/cjson-src/CMakeLists.txt:2 (cmake_minimum_required):
  Compatibility with CMake < 3.5 will be removed from a future version of
  CMake.

  Update the VERSION argument <min> value or use a ...<max> suffix to tell
  CMake that the project does not need compatibility with older versions.


-- Performing Test FLAG_SUPPORTED_fvisibilityhidden
-- Performing Test FLAG_SUPPORTED_fvisibilityhidden - Success
-- Configuring done (6.4s)
-- Generating done (0.2s)
-- Build files have been written to: /Users/bbrock/src/binsparse-reference-c/build
bbrock@rjohns3-mobl2:~/src/binsparse-reference-c$ cd build/examples/
bbrock@rjohns3-mobl2:~/src/binsparse-reference-c/build/examples$ make
[  8%] Building C object _deps/cjson-build/CMakeFiles/cjson.dir/cJSON.c.o
[ 16%] Linking C shared library libcjson.dylib
ld: warning: search path '/opt/homebrew/Cellar/fmt/10.1.1/lib' not found
[ 16%] Built target cjson
[ 25%] Building C object examples/CMakeFiles/simple_matrix_write.dir/simple_matrix_write.c.o
[ 33%] Linking C executable simple_matrix_write
ld: warning: search path '/opt/homebrew/Cellar/fmt/10.1.1/lib' not found
[ 33%] Built target simple_matrix_write
[ 41%] Building C object examples/CMakeFiles/simple_matrix_read.dir/simple_matrix_read.c.o
[ 50%] Linking C executable simple_matrix_read
ld: warning: search path '/opt/homebrew/Cellar/fmt/10.1.1/lib' not found
[ 50%] Built target simple_matrix_read
[ 58%] Building C object examples/CMakeFiles/simple_read.dir/simple_read.c.o
[ 66%] Linking C executable simple_read
ld: warning: search path '/opt/homebrew/Cellar/fmt/10.1.1/lib' not found
[ 66%] Built target simple_read
[ 75%] Building C object examples/CMakeFiles/simple_write.dir/simple_write.c.o
[ 83%] Linking C executable simple_write
ld: warning: search path '/opt/homebrew/Cellar/fmt/10.1.1/lib' not found
[ 83%] Built target simple_write
[ 91%] Building C object examples/CMakeFiles/convert_binsparse.dir/convert_binsparse.c.o
[100%] Linking C executable convert_binsparse
ld: warning: search path '/opt/homebrew/Cellar/fmt/10.1.1/lib' not found
[100%] Built target convert_binsparse
```
