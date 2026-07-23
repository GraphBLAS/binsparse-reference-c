#!/bin/bash

# SPDX-FileCopyrightText: 2024 Binsparse Developers
#
# SPDX-License-Identifier: BSD-3-Clause

# compile_octave.sh - Build Binsparse Octave MEX functions from command line
#
# This script compiles the Binsparse MEX functions for Octave using mkoctfile
# from the command line, without needing to start Octave first.
#
# Usage:
#   ./compile_octave.sh
#   ./compile_octave.sh --verbose
#   ./compile_octave.sh --clean

set -e  # Exit on any error

# Color output for better readability
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Function to print colored output
print_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Parse command line arguments
VERBOSE=false
CLEAN=false

for arg in "$@"; do
    case $arg in
        --verbose|-v)
            VERBOSE=true
            shift
            ;;
        --clean|-c)
            CLEAN=true
            shift
            ;;
        --help|-h)
            echo "Usage: $0 [--verbose] [--clean] [--help]"
            echo "  --verbose, -v: Enable verbose output"
            echo "  --clean, -c:   Clean compiled MEX files"
            echo "  --help, -h:    Show this help message"
            exit 0
            ;;
        *)
            print_error "Unknown option: $arg"
            echo "Use --help for usage information"
            exit 1
            ;;
    esac
done

print_info "Binsparse Octave MEX Compilation Script"
echo "========================================"

# Check if mkoctfile is available
if ! command -v mkoctfile &> /dev/null; then
    print_error "mkoctfile not found. Please install GNU Octave."
    exit 1
fi

mkoctfile_version=$(mkoctfile --version | head -n1)
print_info "Found mkoctfile: $mkoctfile_version"

# Get script directory
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
print_info "Working directory: $SCRIPT_DIR"

# Find Binsparse include directory
BINSPARSE_ROOT="$(realpath "$SCRIPT_DIR/../..")"
INCLUDE_DIR="$BINSPARSE_ROOT/include"

if [ ! -d "$INCLUDE_DIR" ]; then
    print_error "Binsparse include directory not found: $INCLUDE_DIR"
    print_error "Make sure you're running this script from the bindings/matlab directory"
    exit 1
fi

if [ ! -f "$INCLUDE_DIR/binsparse/binsparse.h" ]; then
    print_error "Main Binsparse header not found: $INCLUDE_DIR/binsparse/binsparse.h"
    exit 1
fi

print_info "Using Binsparse include directory: $INCLUDE_DIR"

HDF5_INCLUDE_DIR=""
if [ -d /usr/include/hdf5/serial ]; then
    HDF5_INCLUDE_DIR="/usr/include/hdf5/serial"
elif [ -d /usr/include/hdf5 ]; then
    HDF5_INCLUDE_DIR="/usr/include/hdf5"
fi

if [ -n "$HDF5_INCLUDE_DIR" ]; then
    print_info "Using HDF5 include directory: $HDF5_INCLUDE_DIR"
fi

# Change to script directory
cd "$SCRIPT_DIR"

# Clean function
clean_mex_files() {
    print_info "Cleaning compiled MEX files..."

    # MEX file extensions for different platforms
    extensions=("mex" "mexa64" "mexw32" "mexw64" "mexmaci64")

    found_files=0
    for ext in "${extensions[@]}"; do
        if ls *.${ext} 1> /dev/null 2>&1; then
            for file in *.${ext}; do
                print_info "Removing $file"
                rm "$file"
                ((found_files++))
            done
        fi
    done

    if [ $found_files -eq 0 ]; then
        print_info "No MEX files found to clean"
    else
        print_success "Cleaned $found_files MEX file(s)"
    fi
}

# Handle clean option
if [ "$CLEAN" = true ]; then
    clean_mex_files
    exit 0
fi

# List of MEX files to compile
MEX_FILES=("binsparse_read.c" "binsparse_write.c" "binsparse_from_ssmc.c" "binsparse_minimize_types.c" "binsparse_write_string_dataset.c")

print_info "Compiling MEX functions..."

# Compile each MEX file
for mex_file in "${MEX_FILES[@]}"; do
    if [ ! -f "$mex_file" ]; then
        print_warning "MEX source file not found: $mex_file"
        continue
    fi

    print_info "Compiling $mex_file..."

        # Build mkoctfile command with library linking
    LIB_DIR="$BINSPARSE_ROOT/build"
    LIB_PATH="$LIB_DIR/libbinsparse.a"
    CJSON_LIB_DIR="$LIB_DIR/_deps/cjson-build"

    INCLUDE_FLAGS="-I\"$INCLUDE_DIR\""
    if [ -n "$HDF5_INCLUDE_DIR" ]; then
        INCLUDE_FLAGS="$INCLUDE_FLAGS -I\"$HDF5_INCLUDE_DIR\""
    fi

    CMD="mkoctfile --mex -fPIC $INCLUDE_FLAGS $mex_file -Wl,--whole-archive \"$LIB_PATH\" -Wl,--no-whole-archive -L\"$CJSON_LIB_DIR\" -lcjson -lhdf5_serial"

    if [ "$VERBOSE" = true ]; then
        CMD="$CMD --verbose"
        print_info "Command: $CMD"
    fi

    # Execute compilation
    if eval $CMD; then
        print_success "Successfully compiled $mex_file"
    else
        print_error "Failed to compile $mex_file"
        exit 1
    fi
done

print_success "All MEX functions compiled successfully!"
echo ""
print_info "To test the functions, start Octave and run:"
echo "  test_binsparse_read()"
echo "  test_binsparse_write()"
echo ""
print_info "Or test from command line:"
echo "  octave --eval \"test_binsparse_read()\""
