<!--
SPDX-FileCopyrightText: 2024 Binsparse Developers

SPDX-License-Identifier: BSD-3-Clause
-->

# Binsparse MATLAB/Octave Bindings

This directory contains MATLAB and GNU Octave MEX bindings for the Binsparse C library, providing a simple interface to access Binsparse functionality from both MATLAB and Octave.

## Quick Start

### Prerequisites

**For MATLAB:**
1. **MATLAB** with MEX compiler support
2. **Binsparse C library** headers (included in this repository)

**For Octave:**
1. **GNU Octave** with mkoctfile (usually included)
2. **C compiler** (gcc recommended)
3. **Binsparse C library** headers (included in this repository)

### Setup MEX Compiler (if needed)

**For MATLAB:**
If you haven't configured a MEX compiler yet:

```matlab
mex -setup
```

Choose a compatible C compiler when prompted.

**For Octave:**
Octave usually comes with mkoctfile pre-configured. Verify it works:

```bash
mkoctfile --version
```

### Build the Bindings

#### Option 1: MATLAB

1. Open MATLAB and navigate to this directory:
   ```matlab
   cd('path/to/binsparse-reference-c/bindings/matlab')
   ```

2. Build the MEX functions:
   ```matlab
   build_matlab_bindings()
   ```

3. Test the installation:
   ```matlab
   test_binsparse_read()
   test_binsparse_write()
   test_write_binsparse_from_matlab()
   ```

#### Option 2: Octave (from within Octave)

1. Open Octave and navigate to this directory:
   ```octave
   cd('path/to/binsparse-reference-c/bindings/matlab')
   ```

2. Build the MEX functions:
   ```octave
   build_octave_bindings()
   ```

3. Test the installation:
   ```octave
   test_binsparse_read()
   test_binsparse_write()
   test_write_binsparse_from_matlab()
   ```

#### Option 3: Octave (from command line)

1. Navigate to this directory:
   ```bash
   cd path/to/binsparse-reference-c/bindings/matlab
   ```

2. Build using the shell script:
   ```bash
   ./compile_octave.sh
   ```

3. Test the installation:
   ```bash
   octave --eval "test_binsparse_read()"
   octave --eval "test_binsparse_write()"
   octave --eval "test_write_binsparse_from_matlab()"
   ```

## Usage Examples

### Reading Binsparse Files

**In MATLAB or Octave:**

```matlab
% Read a Binsparse matrix file
matrix = binsparse_read('path/to/matrix.bsp.h5');

% Read from a specific group
matrix = binsparse_read('path/to/matrix.bsp.h5', 'group_name');

% Matrix will be a struct with fields:
% - values: array of matrix values
% - indices_0, indices_1: row/column indices
% - pointers_to_1: pointer array (for CSR/CSC formats)
% - nrows, ncols, nnz: matrix dimensions
% - is_iso: boolean for iso-value matrices
% - format: string ('COO', 'CSR', 'CSC', etc.)
% - structure: string ('general', 'symmetric', etc.)
```

### Writing Binsparse Files

```matlab
% Create a matrix struct (example: 3x3 COO matrix)
matrix = struct();
matrix.values = [1.0; 2.0; 3.0];
matrix.indices_0 = uint64([0; 1; 2]);   % 0-based row indices
matrix.indices_1 = uint64([0; 1; 2]);   % 0-based col indices
matrix.pointers_to_1 = uint64([]);      % Empty for COO
matrix.nrows = 3;
matrix.ncols = 3;
matrix.nnz = 3;
matrix.is_iso = false;
matrix.format = 'COO';
matrix.structure = 'general';

% Write to file
binsparse_write('output.bsp.h5', matrix);

% Write with optional parameters
binsparse_write('output.bsp.h5', matrix, 'my_group');
binsparse_write('output.bsp.h5', matrix, 'my_group', '{"author": "me"}');
binsparse_write('output.bsp.h5', matrix, 'my_group', '{"author": "me"}', 6);
```

### Writing from SuiteSparse Matrix Collection Format

```matlab
% Create a SuiteSparse Matrix Collection problem struct
Problem = struct();
Problem.name = 'test_matrix';
Problem.A = sparse([1 2 3], [1 2 3], [1.5 2.5 3.5], 4, 4);
Problem.title = 'Test Matrix';
Problem.kind = 'artificial/test';

% Write directly from SuiteSparse format to Binsparse
write_binsparse_from_matlab(Problem, 'output.bsp.h5');

% Write with optional parameters
write_binsparse_from_matlab(Problem, 'output.bsp.h5', 'my_group');
write_binsparse_from_matlab(Problem, 'output.bsp.h5', 'my_group', '{"test": "metadata"}');
write_binsparse_from_matlab(Problem, 'output.bsp.h5', 'my_group', '{"test": "metadata"}', 6);
```

### Error Handling

The MEX functions include proper error handling:

```matlab
try
    matrix = binsparse_read('nonexistent_file.bsp.h5')
catch ME
    fprintf('Error: %s\n', ME.message)
end
```

## Files Description

| File | Description |
|------|-------------|
| `binsparse_read.c` | MEX function for reading Binsparse matrix files |
| `binsparse_write.c` | MEX function for writing Binsparse matrix files |
| `write_binsparse_from_matlab.c` | MEX function for writing from SuiteSparse Matrix Collection format |
| `build_matlab_bindings.m` | Main build script for MATLAB MEX functions |
| `build_octave_bindings.m` | Main build script for Octave MEX functions |
| `compile_binsparse_read.m` | Simple compilation script for read function (MATLAB) |
| `compile_binsparse_write.m` | Simple compilation script for write function (MATLAB) |
| `compile_write_binsparse_from_matlab.m` | Simple compilation script for SuiteSparse write function (MATLAB) |
| `compile_binsparse_read_octave.m` | Simple compilation script for read function (Octave) |
| `compile_binsparse_write_octave.m` | Simple compilation script for write function (Octave) |
| `compile_write_binsparse_from_matlab_octave.m` | Simple compilation script for SuiteSparse write function (Octave) |
| `compile_octave.sh` | Shell script for building Octave MEX functions |
| `test_binsparse_read.m` | Test script for read functionality |
| `test_binsparse_write.m` | Test script for write functionality |
| `test_write_binsparse_from_matlab.m` | Test script for SuiteSparse write functionality |
| `bsp_matrix_create.m` | Utility function for creating matrix structs |
| `bsp_matrix_info.m` | Utility function for displaying matrix information |
| `README.md` | This documentation file |

## Technical Details

### MEX Function Structure

The Binsparse MEX functions demonstrate:

1. **Header inclusion**: Proper inclusion of `<binsparse/binsparse.h>`
2. **Type conversion**: Complete mapping between MATLAB and Binsparse types
3. **Error handling**: Using Binsparse error types (`bsp_error_t`)
4. **Memory management**: Safe allocation and cleanup
5. **MATLAB interface**: Proper MEX function structure with validation

### Build Process

**MATLAB build process:**

1. Locates Binsparse include directory (`../../include/`)
2. Compiles MEX functions using MATLAB's `mex` command
3. Links against MATLAB MEX libraries
4. Validates compilation with test functions

**Octave build process:**

1. Locates Binsparse include directory (`../../include/`)
2. Compiles MEX functions using `mkoctfile --mex`
3. Links against Octave MEX libraries
4. Validates compilation with test functions

## Extending the Bindings

To add new Binsparse functionality:

1. Create a new `.c` file with MEX function structure
2. Include `<binsparse/binsparse.h>` and relevant headers
3. Add the filename to `mex_files` list in `build_matlab_bindings.m`
4. Create corresponding test functions

### Example MEX Function Template

```c
#include "mex.h"
#include <binsparse/binsparse.h>

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) {
    // Input validation
    if (nrhs != 1) {
        mexErrMsgIdAndTxt("BinSparse:InvalidInput", "Expected 1 input argument");
    }

    // Call Binsparse functions
    bsp_error_t error = /* your_binsparse_function() */;

    // Handle errors
    if (error != BSP_SUCCESS) {
        mexErrMsgIdAndTxt("BinSparse:Error", "%s", bsp_get_error_string(error));
    }

    // Return results to MATLAB
    plhs[0] = /* create_matlab_output() */;
}
```

## Troubleshooting

### Common Issues

1. **MEX compiler not found**
   - **MATLAB**: Run `mex -setup` to configure a compiler
   - **Octave**: Install mkoctfile (usually comes with Octave)
   - Ensure you have a compatible C compiler installed

2. **Include paths not found**
   - Verify you're running from the `bindings/matlab` directory
   - Check that `../../include/binsparse/binsparse.h` exists

3. **Compilation errors**
   - **MATLAB**: Try building with verbose output: `build_matlab_bindings('verbose')`
   - **Octave**: Try building with verbose output: `build_octave_bindings('verbose')` or `./compile_octave.sh --verbose`
   - Check compiler compatibility with your MATLAB/Octave version

### Platform-Specific Notes

- **Windows**:
  - MATLAB: Microsoft Visual Studio or compatible compiler
  - Octave: MinGW-w64 (often included with Octave installer)
- **macOS**: Xcode command line tools required for both MATLAB and Octave
- **Linux**: GCC or compatible compiler should work for both MATLAB and Octave

## Development Status

This provides complete MATLAB/Octave bindings for Binsparse. Currently implemented:

- ✅ Matrix reading (`binsparse_read`)
- ✅ Matrix writing (`binsparse_write`)
- ✅ Complete type support (all Binsparse types including complex numbers)
- ✅ Optional parameters (groups, JSON metadata, compression)
- ✅ Comprehensive error handling
- ✅ Build system and testing framework
- ✅ Round-trip compatibility (read → write → read)
- ⏳ Tensor support (future work)
- ⏳ Advanced sparse matrix operations (future work)

## License

This code is licensed under the BSD-3-Clause license, same as the main Binsparse project.
