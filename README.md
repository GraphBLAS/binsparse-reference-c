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
  bsp_matrix_t mat;
  bsp_read_matrix(&mat, "chesapeake.bsp.hdf5", NULL);

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

  bsp_destroy_matrix_t(&mat);
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

A simple top-level Makefile can be used to compile, test, and install
binsparse:

    make
    make tests
    sudu make install

To remove all compiled files and libraries:

    make distclean

FIXME: the above will segfault because assert(...) is removed for Release.
For now use:

    make debug
    make tests

