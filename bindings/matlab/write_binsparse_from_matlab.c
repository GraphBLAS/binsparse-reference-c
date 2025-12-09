/*
 * SPDX-FileCopyrightText: 2024 Binsparse Developers
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/**
 * write_binsparse_from_matlab.c - Write SuiteSparse Matrix Collection Problem
 * struct to Binsparse format
 *
 * This MEX function takes a SuiteSparse Matrix Collection problem struct and
 * converts it to the Binsparse format, writing the result to a specified file.
 *
 * Usage in MATLAB/Octave:
 *   write_binsparse_from_matlab(problem_struct, filename)
 *   write_binsparse_from_matlab(problem_struct, filename, group)
 *   write_binsparse_from_matlab(problem_struct, filename, group, json_metadata)
 *   write_binsparse_from_matlab(problem_struct, filename, group, json_metadata,
 * compression_level)
 *
 * Arguments:
 *   problem_struct - SuiteSparse Matrix Collection problem struct with fields:
 *      .name - Problem name (string)
 *      .A - Sparse matrix (MATLAB sparse matrix)
 *      .Zeros - Sparse matrix with pattern of explicit zeros in the problem
 *      .title - Problem title (optional)
 *      .kind - Problem kind (optional)
 *      .notes - Additional notes (optional)
 *   filename          - Output filename for the Binsparse file
 *   group             - Optional HDF5 group name (default: 'default')
 *   json_metadata     - Optional JSON metadata string
 *   compression_level - Optional compression level (0-9, default: 1)
 */

#include "mex.h"
#include <binsparse/binsparse.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#if 0
static inline void* bsp_matlab_malloc(size_t size) {
  void* ptr = mxMalloc(size);
  mexMakeMemoryPersistent(ptr);
  return ptr;
}
#endif

static const bsp_allocator_t bsp_matlab_allocator = {
    .malloc = mxMalloc, .free = mxFree};

typedef struct {
  double* values;
  mwIndex* rowind;
  mwIndex* colptr;
  size_t nrows;
  size_t ncols;
  size_t nnz;
} matlab_csc_t;

int extract_matlab_csc(const mxArray* mx_matrix, matlab_csc_t* csc_matrix) {
  // Validate input
  if (!mx_matrix) {
    mexPrintf("Error: NULL matrix pointer\n");
    return -1;
  }

  if (!mxIsSparse(mx_matrix)) {
    mexPrintf("Error: Matrix is not sparse\n");
    return -1;
  }

  if (mxIsComplex(mx_matrix)) {
    mexPrintf("Error: Complex matrices not yet supported\n");
    return -1;
  }

  // Extract matrix dimensions
  csc_matrix->nrows = mxGetM(mx_matrix);
  csc_matrix->ncols = mxGetN(mx_matrix);
  csc_matrix->nnz = mxGetNzmax(mx_matrix);

  // Get pointers to MATLAB's internal CSC data
  // Note: MATLAB stores sparse matrices in CSC format internally
  csc_matrix->values = mxGetPr(mx_matrix); // Non-zero values
  csc_matrix->rowind = mxGetIr(mx_matrix); // Row indices (0-based)
  csc_matrix->colptr = mxGetJc(mx_matrix); // Column pointers

  // Validate that we got valid pointers
  if (!csc_matrix->values || !csc_matrix->rowind || !csc_matrix->colptr) {
    mexPrintf("Error: Failed to extract CSC data from MATLAB matrix\n");
    return -1;
  }

  // The actual number of non-zeros might be less than nzmax
  if (csc_matrix->ncols > 0) {
    csc_matrix->nnz =
        csc_matrix->colptr[csc_matrix->ncols]; // Last element of colptr gives
                                               // actual nnz
  }

  mexPrintf("Extracted CSC matrix: %zu x %zu with %zu non-zeros\n",
            csc_matrix->nrows, csc_matrix->ncols, csc_matrix->nnz);

  return 0; // Success
}

bsp_matrix_t merge_csc_with_zeros(matlab_csc_t matrix, matlab_csc_t zeros) {

  mexPrintf("Merging %zu x %zu matrix (%zu nnz) with zeros pattern (%zu nnz)\n",
            matrix.nrows, matrix.ncols, matrix.nnz, zeros.nnz);

  // If there are no zeros, just construct matrix based on `matrix` and return.
  if (zeros.nnz == 0) {
    bsp_matrix_t result;
    bsp_construct_default_matrix_t_allocator(&result, bsp_matlab_allocator);

    result.nrows = matrix.nrows;
    result.ncols = matrix.ncols;
    result.nnz = matrix.nnz;
    result.format = BSP_CSC;
    result.structure = BSP_GENERAL;
    result.is_iso = false;

    bsp_construct_array_t_allocator(&result.values, matrix.nnz, BSP_FLOAT64,
                                    bsp_matlab_allocator);
    bsp_construct_array_t_allocator(&result.pointers_to_1, matrix.ncols + 1,
                                    BSP_UINT64, bsp_matlab_allocator);
    bsp_construct_array_t_allocator(&result.indices_1, matrix.nnz, BSP_UINT64,
                                    bsp_matlab_allocator);

    double* result_values = (double*) result.values.data;
    /*
    OLD:
    mwIndex* result_rowind = (mwIndex*) result.pointers_to_1.data;
    mwIndex* result_colptr = (mwIndex*) result.indices_1.data;
    */
    // NEW:
    mwIndex* result_rowind = (mwIndex*) result.indices_1.data;
    mwIndex* result_colptr = (mwIndex*) result.pointers_to_1.data;

    for (size_t i = 0; i < result.values.size; i++) {
      result_values[i] = matrix.values[i];
    }

    for (size_t i = 0; i < result.pointers_to_1.size; i++) {
      result_colptr[i] = matrix.colptr[i];
    }

    for (size_t i = 0; i < result.indices_1.size; i++) {
      result_rowind[i] = matrix.rowind[i];
    }

    return result;
  }

  // Initialize result, which will hold the merged `matrix` and `zeros`.
  bsp_matrix_t result;
  bsp_construct_default_matrix_t_allocator(&result, bsp_matlab_allocator);

  // Set up result matrix metadata
  result.nrows = matrix.nrows;
  result.ncols = matrix.ncols;
  result.nnz = matrix.nnz + zeros.nnz;
  result.format = BSP_CSC;
  result.structure = BSP_GENERAL;
  result.is_iso = false;

  assert(sizeof(mwIndex) == sizeof(uint64_t));

  bsp_construct_array_t_allocator(&result.values, matrix.nnz + zeros.nnz,
                                  BSP_FLOAT64, bsp_matlab_allocator);
  bsp_construct_array_t_allocator(&result.pointers_to_1, matrix.ncols + 1,
                                  BSP_UINT64, bsp_matlab_allocator);
  bsp_construct_array_t_allocator(&result.indices_1, matrix.nnz + zeros.nnz,
                                  BSP_UINT64, bsp_matlab_allocator);

  double* result_values = (double*) result.values.data;
  mwIndex* result_colptr = (mwIndex*) result.pointers_to_1.data;
  mwIndex* result_rowind = (mwIndex*) result.indices_1.data;

  // Set colptr for result
  result_colptr[0] = 0;

  for (mwIndex j = 1; j < matrix.ncols + 1; j++) {
    mwIndex row_nnz_matrix = matrix.colptr[j] - matrix.colptr[j - 1];
    mwIndex row_nnz_zeros = zeros.colptr[j] - zeros.colptr[j - 1];
    result_colptr[j] = result_colptr[j - 1] + row_nnz_matrix + row_nnz_zeros;
  }

  // FIXME: this produces a bsp_matrix_t with row indices out of order.
  // Is that OK?

  for (mwIndex j = 0; j < result.ncols; j++) {
    mwIndex result_i_ptr = result_colptr[j];

    for (mwIndex matrix_i_ptr = matrix.colptr[j];
         matrix_i_ptr < matrix.colptr[j + 1]; matrix_i_ptr++) {
      result_values[result_i_ptr] = matrix.values[matrix_i_ptr];
      result_rowind[result_i_ptr] = matrix.rowind[matrix_i_ptr];

      result_i_ptr++;
    }

    for (mwIndex zeros_i_ptr = zeros.colptr[j];
         zeros_i_ptr < zeros.colptr[j + 1]; zeros_i_ptr++) {
      result_values[result_i_ptr] = zeros.values[zeros_i_ptr];  // FIXME: should be 0
      result_rowind[result_i_ptr] = zeros.rowind[zeros_i_ptr];

      result_i_ptr++;
    }

    assert(result_i_ptr == result_colptr[j + 1]);
  }

  mexPrintf("Successfully created merged bsp_matrix_t in CSC format\n");
  return result;
}

/**
 * Print information about a SuiteSparse Matrix Collection problem struct
 */
void print_problem_info(const mxArray* problem_struct) {
  mexPrintf("=== SuiteSparse Matrix Collection Problem Information ===\n");

  // Check if input is a struct
  if (!mxIsStruct(problem_struct)) {
    mexPrintf("Error: Input is not a struct\n");
    return;
  }

  mexPrintf("Number of fields: %d\n", mxGetNumberOfFields(problem_struct));
  mexPrintf("Number of elements: %d\n",
            (int) mxGetNumberOfElements(problem_struct));

  // Print field names
  mexPrintf("Field names:\n");
  for (int i = 0; i < mxGetNumberOfFields(problem_struct); i++) {
    const char* field_name = mxGetFieldNameByNumber(problem_struct, i);
    mexPrintf("  [%d] %s\n", i, field_name);

    // Get field value and print basic info
    mxArray* field_value = mxGetFieldByNumber(problem_struct, 0, i);
    if (field_value) {
      if (mxIsChar(field_value)) {
        char* str_value = mxArrayToString(field_value);
        if (str_value) {
          mexPrintf("      (string): \"%s\"\n", str_value);
          mxFree (str_value);
        }
      } else if (mxIsSparse(field_value)) {
        mexPrintf("      (sparse matrix): %dx%d with %d non-zeros\n",
                  (int) mxGetM(field_value), (int) mxGetN(field_value),
                  (int) mxGetNzmax(field_value));
      } else if (mxIsNumeric(field_value)) {
        mexPrintf("      (numeric): %dx%d %s array\n",
                  (int) mxGetM(field_value), (int) mxGetN(field_value),
                  mxGetClassName(field_value));
      } else {
        mexPrintf("      (%s): %dx%d\n", mxGetClassName(field_value),
                  (int) mxGetM(field_value), (int) mxGetN(field_value));
      }
    } else {
      mexPrintf("      (null)\n");
    }
  }
  mexPrintf("=========================================================\n\n");
}

/**
 * Main MEX function entry point
 */
void mexFunction(int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[]) {
  char* filename = NULL;
  char* group = NULL;
  char* json_metadata = NULL;
  int compression_level = 0; // Default compression

  // Check input arguments
  if (nrhs < 2 || nrhs > 5) {
    mexErrMsgIdAndTxt(
        "BinSparse:InvalidArgs",
        "Usage: write_binsparse_from_matlab(problem_struct, filename [, group "
        "[, json_metadata [, compression_level]]])");
  }

  if (nlhs > 0) {
    mexErrMsgIdAndTxt("BinSparse:TooManyOutputs",
                      "No output arguments expected");
  }

  mexPrintf("Number of input arguments: %d\n", nrhs);
  mexPrintf("Number of output arguments: %d\n", nlhs);

  // Validate and process problem struct (first argument)
  if (!mxIsStruct(prhs[0])) {
    mexErrMsgIdAndTxt("BinSparse:InvalidProblemStruct",
                      "First argument must be a SuiteSparse Matrix Collection "
                      "problem struct (input is not a struct)");
  }

  // for Octave: the struct contains "Problem", which must be dereferences
  const mxArray* mx_problem = mxGetField(prhs[0], 0, "Problem");
  if (mx_problem == NULL)
  { 
    // for MATLAB: the struct is the Problem already, and contains .A, etc
    mx_problem = prhs [0] ;
  }

  if ((mx_problem == NULL) || !mxIsStruct(mx_problem)) {
    mexErrMsgIdAndTxt("BinSparse:InvalidProblemStruct",
                      "First argument must be a SuiteSparse Matrix Collection "
                      "problem struct (input is not a Problem struct)");
  }

  // Extract sparse matrix from problem.A field
  mxArray* mx_matrix = mxGetField(mx_problem, 0, "A");

  if (!mx_matrix) {
    mexErrMsgIdAndTxt("BinSparse:InvalidProblemStruct",
                      "First argument must be a SuiteSparse Matrix Collection "
                      "problem struct (Problem.A does not exist)");
  }

  if (!mxIsSparse(mx_matrix)) {
    mexErrMsgIdAndTxt(
        "BinSparse:InvalidProblemStruct",
        "Unable to extract Matlab sparse matrix --- not a matrix");
  }

  mexPrintf("Found sparse matrix in problem.A field\n");
  matlab_csc_t csc_matrix = {0};
  int rv = extract_matlab_csc(mx_matrix, &csc_matrix);

  if (rv != 0) {
    mexErrMsgIdAndTxt("BinSparse:InvalidProblemStruct",
                      "Unable to extract Matlab sparse matrix");
  }

  mxArray* mx_zeros_matrix = mxGetField(mx_problem, 0, "Zeros");

  matlab_csc_t zeros_csc_matrix = {0};

  if (mx_zeros_matrix) {
    if (!mxIsSparse(mx_zeros_matrix)) {
      mexErrMsgIdAndTxt("BinSparse:InvalidProblemStruct",
                        "Zeros matrix exists but is not sparse");
    }

    int rv = extract_matlab_csc(mx_zeros_matrix, &zeros_csc_matrix);

    if (rv != 0) {
      mexErrMsgIdAndTxt("BinSparse:InvalidProblemStruct",
                        "Unable to extract Zeros sparse matrix");
    }
  }

  bsp_matrix_t merged_matrix =
      merge_csc_with_zeros(csc_matrix, zeros_csc_matrix);

  mexPrintf("Merged matrix!\n");

  mexPrintf("Merged matrix is %zu x %zu and has %zu nnz.\n",
            merged_matrix.nrows, merged_matrix.ncols, merged_matrix.nnz);

  bsp_destroy_matrix_t(&merged_matrix);

  mexPrintf("\nAnalyzing problem struct:\n");
  print_problem_info(prhs[0]);

  // Get filename (second argument)
  if (!mxIsChar(prhs[1])) {
    mexErrMsgIdAndTxt("BinSparse:InvalidFilename", "Filename must be a string");
  }

  filename = mxArrayToString(prhs[1]);
  if (!filename) {
    mexErrMsgIdAndTxt("BinSparse:MemoryError",
                      "Failed to convert filename string");
  }
  mexPrintf("Output filename: \"%s\"\n", filename);

  // Get optional group name (third argument)
  if (nrhs >= 3 && !mxIsEmpty(prhs[2])) {
    if (!mxIsChar(prhs[2])) {
      mexErrMsgIdAndTxt("BinSparse:InvalidGroup",
                        "Group name must be a string");
    }

    group = mxArrayToString(prhs[2]);
    if (!group) {
      mexErrMsgIdAndTxt("BinSparse:MemoryError",
                        "Failed to convert group string");
    }
    mexPrintf("HDF5 group: \"%s\"\n", group);
  } else {
    mexPrintf("HDF5 group: (default)\n");
  }

  // Get optional JSON metadata (fourth argument)
  if (nrhs >= 4 && !mxIsEmpty(prhs[3])) {
    if (!mxIsChar(prhs[3])) {
      mexErrMsgIdAndTxt("BinSparse:InvalidJSON",
                        "JSON metadata must be a string");
    }

    json_metadata = mxArrayToString(prhs[3]);
    if (!json_metadata) {
      mexErrMsgIdAndTxt("BinSparse:MemoryError",
                        "Failed to convert JSON metadata string");
    }
    mexPrintf("JSON metadata: \"%s\"\n", json_metadata);
  } else {
    mexPrintf("JSON metadata: (none)\n");
  }

  // Get optional compression level (fifth argument)
  if (nrhs >= 5 && !mxIsEmpty(prhs[4])) {
    if (!mxIsNumeric(prhs[4]) || mxIsComplex(prhs[4]) ||
        mxGetNumberOfElements(prhs[4]) != 1) {
      mexErrMsgIdAndTxt("BinSparse:InvalidCompression",
                        "Compression level must be a scalar integer");
    }

    compression_level = (int) mxGetScalar(prhs[4]);
    mexPrintf("Compression level: %d\n", compression_level);
  } else {
    mexPrintf("Compression level: %d (default)\n", compression_level);
  }

  mexPrintf("\n=== IMPLEMENTATION STATUS ===\n");

  // Extract sparse matrix from problem.A field
  mxArray* matrix_field = mxGetField(prhs[0], 0, "A");
  if (matrix_field && mxIsSparse(matrix_field)) {
    mexPrintf("✓ Found sparse matrix in problem.A field\n");

    // Test the matlab_csc_to_bsp function
    mexPrintf("Testing matlab_csc_to_bsp function:\n");
    matlab_csc_t csc;
    extract_matlab_csc(matrix_field, &csc);

    if (csc.values && csc.rowind && csc.colptr) {
      mexPrintf("✓ Successfully extracted CSC data:\n");
      mexPrintf("  - Dimensions: %zu x %zu\n", csc.nrows, csc.ncols);
      mexPrintf("  - Non-zeros: %zu\n", csc.nnz);
      mexPrintf("  - Values pointer: %p\n", (void*) csc.values);
      mexPrintf("  - Row indices pointer: %p\n", (void*) csc.rowind);
      mexPrintf("  - Column pointers: %p\n", (void*) csc.colptr);

      // Show first few values as example
      if (csc.nnz > 0) {
        mexPrintf("  - First few values: ");
        size_t show_count = (csc.nnz < 5) ? csc.nnz : 5;
        for (size_t i = 0; i < show_count; i++) {
          mexPrintf("%.6g ", csc.values[i]);
        }
        if (csc.nnz > 5)
          mexPrintf("...");
        mexPrintf("\n");
      }
    } else {
      mexPrintf("✗ Failed to extract CSC data\n");
    }
  } else {
    mexPrintf("✗ No sparse matrix found in problem.A field\n");
  }

  mexPrintf("\nTODO: Convert MATLAB sparse matrix to bsp_matrix_t\n");
  mexPrintf("TODO: Add metadata from problem struct to JSON\n");
  mexPrintf("TODO: Call bsp_write_matrix() to write the file\n");
  mexPrintf("=====================================\n\n");

  mexPrintf("Function completed successfully (skeleton mode)\n");

  // Clean up allocated strings
  mxFree (json_metadata);
  mxFree (group);
  mxFree (filename);
}
