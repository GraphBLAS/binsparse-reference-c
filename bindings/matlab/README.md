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
   test_bsp_hello()
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
   test_bsp_hello_octave()
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
   octave --eval "test_bsp_hello_octave()"
   ```

## Usage Examples

### Basic Usage

**In MATLAB or Octave:**

```matlab
% Simple greeting
result = bsp_hello()
% Output: 'Binsparse MEX binding is working!'

% Get Binsparse version
[version, success] = bsp_hello('version')
% Output: version = '0.1', success = true
```

### Error Handling

The MEX functions include proper error handling:

```matlab
try
    result = bsp_hello('invalid_mode')
catch ME
    fprintf('Error: %s\n', ME.message)
end
```

## Files Description

| File | Description |
|------|-------------|
| `bsp_hello.c` | Simple MEX function demonstrating Binsparse integration |
| `build_matlab_bindings.m` | Main build script for MATLAB MEX functions |
| `build_octave_bindings.m` | Main build script for Octave MEX functions |
| `compile_bsp_hello.m` | Simple compilation script for the demo function (MATLAB) |
| `compile_octave.sh` | Shell script for building Octave MEX functions |
| `test_bsp_hello.m` | Test script to verify functionality (MATLAB) |
| `test_bsp_hello_octave.m` | Test script to verify functionality (Octave) |
| `README.md` | This documentation file |

## Technical Details

### MEX Function Structure

The `bsp_hello` MEX function demonstrates:

1. **Header inclusion**: Proper inclusion of `<binsparse/binsparse.h>`
2. **Error handling**: Using Binsparse error types (`bsp_error_t`)
3. **Memory management**: Safe allocation and cleanup
4. **MATLAB interface**: Proper MEX function structure

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

This is a minimal demonstration of MATLAB/Octave bindings for Binsparse. Currently implemented:

- ✅ Basic MEX function structure
- ✅ Binsparse header inclusion
- ✅ Error handling with Binsparse error types
- ✅ Build system and testing framework
- ⏳ Matrix reading/writing functions (future work)
- ⏳ Advanced Binsparse features (future work)

## License

This code is licensed under the BSD-3-Clause license, same as the main Binsparse project.
