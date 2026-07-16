/* SPDX-FileCopyrightText: 2024 Binsparse Developers
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MATLAB_BSP_HELPERS_H
#define MATLAB_BSP_HELPERS_H

#include "mex.h"
#include <binsparse/binsparse.h>
#include <complex.h>
#include <string.h>

typedef struct {
  double* values;
  double* imag_values;
  mwIndex* rowind;
  mwIndex* colptr;
  size_t nrows;
  size_t ncols;
  size_t nnz;
  bool has_values;
  bool is_complex;
} matlab_csc_t;

static const bsp_allocator_t bsp_matlab_allocator = {.malloc = mxMalloc,
                                                     .free = mxFree};

static inline int extract_matlab_csc(const mxArray* mx_matrix,
                                     matlab_csc_t* csc_matrix) {
  if (!mx_matrix || !csc_matrix) {
    return -1;
  }

  if (!mxIsSparse(mx_matrix)) {
    return -1;
  }

  if (!mxIsDouble(mx_matrix) && !mxIsLogical(mx_matrix)) {
    return -1;
  }

  if (mxIsLogical(mx_matrix) && mxIsComplex(mx_matrix)) {
    return -1;
  }

  csc_matrix->nrows = mxGetM(mx_matrix);
  csc_matrix->ncols = mxGetN(mx_matrix);

  csc_matrix->has_values = mxIsDouble(mx_matrix);
  csc_matrix->is_complex = mxIsComplex(mx_matrix);
  csc_matrix->values = csc_matrix->has_values ? mxGetPr(mx_matrix) : NULL;
  csc_matrix->imag_values = csc_matrix->is_complex ? mxGetPi(mx_matrix) : NULL;
  csc_matrix->rowind = mxGetIr(mx_matrix);
  csc_matrix->colptr = mxGetJc(mx_matrix);

  if (!csc_matrix->rowind || !csc_matrix->colptr) {
    return -1;
  }

  if (csc_matrix->ncols > 0) {
    csc_matrix->nnz = csc_matrix->colptr[csc_matrix->ncols];
  } else {
    csc_matrix->nnz = 0;
  }

  if (csc_matrix->nnz > 0 && csc_matrix->has_values && !csc_matrix->values) {
    return -1;
  }

  return 0;
}

static inline bsp_error_t
matlab_to_bsp_array_allocator(const mxArray* mx_array, bsp_array_t* array,
                              bsp_allocator_t allocator) {
  bool is_empty = mxIsEmpty(mx_array);

  size_t size = mxGetNumberOfElements(mx_array);
  mxClassID class_id = mxGetClassID(mx_array);
  bool is_complex = mxIsComplex(mx_array);

  bsp_type_t bsp_type;
  size_t element_size;

  if (is_complex) {
    if (class_id == mxDOUBLE_CLASS) {
      bsp_type = BSP_COMPLEX_FLOAT64;
      element_size = sizeof(double _Complex);
    } else if (class_id == mxSINGLE_CLASS) {
      bsp_type = BSP_COMPLEX_FLOAT32;
      element_size = sizeof(float _Complex);
    } else {
      return BSP_INVALID_TYPE;
    }
  } else {
    switch (class_id) {
    case mxDOUBLE_CLASS:
      bsp_type = BSP_FLOAT64;
      element_size = sizeof(double);
      break;
    case mxSINGLE_CLASS:
      bsp_type = BSP_FLOAT32;
      element_size = sizeof(float);
      break;
    case mxUINT64_CLASS:
      bsp_type = BSP_UINT64;
      element_size = sizeof(uint64_t);
      break;
    case mxUINT32_CLASS:
      bsp_type = BSP_UINT32;
      element_size = sizeof(uint32_t);
      break;
    case mxUINT16_CLASS:
      bsp_type = BSP_UINT16;
      element_size = sizeof(uint16_t);
      break;
    case mxUINT8_CLASS:
      bsp_type = BSP_UINT8;
      element_size = sizeof(uint8_t);
      break;
    case mxINT64_CLASS:
      bsp_type = BSP_INT64;
      element_size = sizeof(int64_t);
      break;
    case mxINT32_CLASS:
      bsp_type = BSP_INT32;
      element_size = sizeof(int32_t);
      break;
    case mxINT16_CLASS:
      bsp_type = BSP_INT16;
      element_size = sizeof(int16_t);
      break;
    case mxINT8_CLASS:
      bsp_type = BSP_INT8;
      element_size = sizeof(int8_t);
      break;
    default:
      return BSP_INVALID_TYPE;
    }
  }

  if (is_empty) {
    array->data = NULL;
    array->size = 0;
    array->type = bsp_type;
    array->allocator = allocator;
    return BSP_SUCCESS;
  }

  bsp_error_t error =
      bsp_construct_array_t_allocator(array, size, bsp_type, allocator);
  if (error != BSP_SUCCESS) {
    return error;
  }

  if (is_complex) {
    if (class_id == mxDOUBLE_CLASS) {
      double* real_data = mxGetPr(mx_array);
      double* imag_data = mxGetPi(mx_array);
      double _Complex* out_data = (double _Complex*) array->data;
      for (size_t i = 0; i < size; i++) {
        double imag = imag_data ? imag_data[i] : 0.0;
        out_data[i] = real_data[i] + imag * I;
      }
    } else {
      float* real_data = (float*) mxGetData(mx_array);
      float* imag_data = (float*) mxGetImagData(mx_array);
      float _Complex* out_data = (float _Complex*) array->data;
      for (size_t i = 0; i < size; i++) {
        float imag = imag_data ? imag_data[i] : 0.0f;
        out_data[i] = real_data[i] + imag * I;
      }
    }
  } else {
    memcpy(array->data, mxGetData(mx_array), size * element_size);
  }

  return BSP_SUCCESS;
}

static inline bsp_error_t matlab_struct_to_bsp_matrix_allocator(
    const mxArray* mx_struct, bsp_matrix_t* matrix, bsp_allocator_t allocator) {
  bsp_construct_default_matrix_t_allocator(matrix, allocator);

  mxArray* values_field = mxGetField(mx_struct, 0, "values");
  mxArray* indices_0_field = mxGetField(mx_struct, 0, "indices_0");
  mxArray* indices_1_field = mxGetField(mx_struct, 0, "indices_1");
  mxArray* pointers_to_1_field = mxGetField(mx_struct, 0, "pointers_to_1");

  if (!values_field || !indices_0_field || !indices_1_field ||
      !pointers_to_1_field) {
    bsp_destroy_matrix_t(matrix);
    return BSP_INVALID_STRUCTURE;
  }

  bsp_error_t error =
      matlab_to_bsp_array_allocator(values_field, &matrix->values, allocator);
  if (error != BSP_SUCCESS) {
    bsp_destroy_matrix_t(matrix);
    return error;
  }

  error = matlab_to_bsp_array_allocator(indices_0_field, &matrix->indices_0,
                                        allocator);
  if (error != BSP_SUCCESS) {
    bsp_destroy_matrix_t(matrix);
    return error;
  }

  error = matlab_to_bsp_array_allocator(indices_1_field, &matrix->indices_1,
                                        allocator);
  if (error != BSP_SUCCESS) {
    bsp_destroy_matrix_t(matrix);
    return error;
  }

  error = matlab_to_bsp_array_allocator(pointers_to_1_field,
                                        &matrix->pointers_to_1, allocator);
  if (error != BSP_SUCCESS) {
    bsp_destroy_matrix_t(matrix);
    return error;
  }

  mxArray* nrows_field = mxGetField(mx_struct, 0, "nrows");
  mxArray* ncols_field = mxGetField(mx_struct, 0, "ncols");
  mxArray* nnz_field = mxGetField(mx_struct, 0, "nnz");
  mxArray* is_iso_field = mxGetField(mx_struct, 0, "is_iso");

  if (!nrows_field || !ncols_field || !nnz_field || !is_iso_field) {
    bsp_destroy_matrix_t(matrix);
    return BSP_INVALID_STRUCTURE;
  }

  matrix->nrows = (size_t) mxGetScalar(nrows_field);
  matrix->ncols = (size_t) mxGetScalar(ncols_field);
  matrix->nnz = (size_t) mxGetScalar(nnz_field);
  matrix->is_iso = mxIsLogicalScalarTrue(is_iso_field);

  mxArray* format_field = mxGetField(mx_struct, 0, "format");
  if (!format_field || !mxIsChar(format_field)) {
    bsp_destroy_matrix_t(matrix);
    return BSP_INVALID_STRUCTURE;
  }

  char* format_str = mxArrayToString(format_field);
  if (!format_str) {
    bsp_destroy_matrix_t(matrix);
    return BSP_INVALID_STRUCTURE;
  }

  matrix->format = bsp_get_matrix_format(format_str);
  mxFree(format_str);

  if (matrix->format == BSP_INVALID_FORMAT) {
    bsp_destroy_matrix_t(matrix);
    return BSP_INVALID_FORMAT;
  }

  mxArray* structure_field = mxGetField(mx_struct, 0, "structure");
  if (!structure_field || !mxIsChar(structure_field)) {
    bsp_destroy_matrix_t(matrix);
    return BSP_INVALID_STRUCTURE;
  }

  char* structure_str = mxArrayToString(structure_field);
  if (!structure_str) {
    bsp_destroy_matrix_t(matrix);
    return BSP_INVALID_STRUCTURE;
  }

  matrix->structure = bsp_get_structure(structure_str);
  mxFree(structure_str);

  if (matrix->structure == BSP_INVALID_STRUCTURE) {
    matrix->structure = BSP_GENERAL;
  }

  return BSP_SUCCESS;
}

static inline bsp_error_t
bsp_matrix_copy_with_allocator(const bsp_matrix_t* input, bsp_matrix_t* output,
                               bsp_allocator_t allocator) {
  bsp_construct_default_matrix_t_allocator(output, allocator);

  output->nrows = input->nrows;
  output->ncols = input->ncols;
  output->nnz = input->nnz;
  output->is_iso = input->is_iso;
  output->format = input->format;
  output->structure = input->structure;
  output->values.type = input->values.type;
  output->indices_0.type = input->indices_0.type;
  output->indices_1.type = input->indices_1.type;
  output->pointers_to_1.type = input->pointers_to_1.type;

  if (input->values.size > 0) {
    bsp_error_t error = bsp_construct_array_t_allocator(
        &output->values, input->values.size, input->values.type, allocator);
    if (error != BSP_SUCCESS) {
      bsp_destroy_matrix_t(output);
      return error;
    }
    memcpy(output->values.data, input->values.data,
           input->values.size * bsp_type_size(input->values.type));
  }

  if (input->indices_0.size > 0) {
    bsp_error_t error = bsp_construct_array_t_allocator(
        &output->indices_0, input->indices_0.size, input->indices_0.type,
        allocator);
    if (error != BSP_SUCCESS) {
      bsp_destroy_matrix_t(output);
      return error;
    }
    memcpy(output->indices_0.data, input->indices_0.data,
           input->indices_0.size * bsp_type_size(input->indices_0.type));
  }

  if (input->indices_1.size > 0) {
    bsp_error_t error = bsp_construct_array_t_allocator(
        &output->indices_1, input->indices_1.size, input->indices_1.type,
        allocator);
    if (error != BSP_SUCCESS) {
      bsp_destroy_matrix_t(output);
      return error;
    }
    memcpy(output->indices_1.data, input->indices_1.data,
           input->indices_1.size * bsp_type_size(input->indices_1.type));
  }

  if (input->pointers_to_1.size > 0) {
    bsp_error_t error = bsp_construct_array_t_allocator(
        &output->pointers_to_1, input->pointers_to_1.size,
        input->pointers_to_1.type, allocator);
    if (error != BSP_SUCCESS) {
      bsp_destroy_matrix_t(output);
      return error;
    }
    memcpy(output->pointers_to_1.data, input->pointers_to_1.data,
           input->pointers_to_1.size *
               bsp_type_size(input->pointers_to_1.type));
  }

  return BSP_SUCCESS;
}

static inline mxClassID get_mxClassID(bsp_type_t type) {
  switch (type) {
  case BSP_UINT8:
    return mxUINT8_CLASS;
  case BSP_UINT16:
    return mxUINT16_CLASS;
  case BSP_UINT32:
    return mxUINT32_CLASS;
  case BSP_UINT64:
    return mxUINT64_CLASS;
  case BSP_INT8:
    return mxINT8_CLASS;
  case BSP_INT16:
    return mxINT16_CLASS;
  case BSP_INT32:
    return mxINT32_CLASS;
  case BSP_INT64:
    return mxINT64_CLASS;
  case BSP_FLOAT32:
    return mxSINGLE_CLASS;
  case BSP_FLOAT64:
    return mxDOUBLE_CLASS;
  case BSP_BINT8:
    return mxUINT8_CLASS;
  case BSP_COMPLEX_FLOAT32:
    return mxSINGLE_CLASS;
  case BSP_COMPLEX_FLOAT64:
    return mxDOUBLE_CLASS;
  default:
    return mxUNKNOWN_CLASS;
  }
}

static inline mxComplexity get_mxComplexity(bsp_type_t type) {
  if (type == BSP_COMPLEX_FLOAT32 || type == BSP_COMPLEX_FLOAT64) {
    return mxCOMPLEX;
  }
  return mxREAL;
}

static inline mxArray* bsp_array_to_matlab(bsp_array_t* array) {
  if (!array || array->data == NULL || array->size == 0) {
    bsp_type_t type = array ? array->type : BSP_FLOAT64;
    mxClassID class_id = get_mxClassID(type);
    if (class_id == mxUNKNOWN_CLASS) {
      class_id = mxDOUBLE_CLASS;
      type = BSP_FLOAT64;
    }
    mxArray* empty_array =
        mxCreateNumericMatrix(0, 1, class_id, get_mxComplexity(type));
    return empty_array;
  }

  if (get_mxClassID(array->type) == mxUNKNOWN_CLASS) {
    mexWarnMsgIdAndTxt("BinSparse:UnsupportedType",
                       "Unsupported array type %d, returning empty array",
                       (int) array->type);
    mxArray* empty_array = mxCreateNumericMatrix(0, 1, mxDOUBLE_CLASS, mxREAL);
    return empty_array;
  }

  mxArray* mx_array = NULL;

  if ((array->allocator.malloc == bsp_matlab_allocator.malloc &&
       array->allocator.free == bsp_matlab_allocator.free) &&
      get_mxComplexity(array->type) == mxREAL) {
    mx_array = mxCreateNumericMatrix(0, 1, get_mxClassID(array->type),
                                     get_mxComplexity(array->type));

    mxSetData(mx_array, array->data);
    mxSetM(mx_array, array->size);

    array->data = NULL;
    array->size = 0;
  } else {
    mx_array = mxCreateNumericMatrix(array->size, 1, get_mxClassID(array->type),
                                     get_mxComplexity(array->type));

    if (get_mxComplexity(array->type) == mxREAL) {
      memcpy(mxGetData(mx_array), array->data,
             array->size * bsp_type_size(array->type));
    } else {
      if (array->type == BSP_COMPLEX_FLOAT32) {
        float _Complex* in_data = (float _Complex*) array->data;
        float* real_data = (float*) mxGetData(mx_array);
        float* imag_data = (float*) mxGetImagData(mx_array);
        for (size_t i = 0; i < array->size; i++) {
          real_data[i] = crealf(in_data[i]);
          imag_data[i] = cimagf(in_data[i]);
        }
      } else {
        double _Complex* in_data = (double _Complex*) array->data;
        double* real_data = mxGetPr(mx_array);
        double* imag_data = mxGetPi(mx_array);
        for (size_t i = 0; i < array->size; i++) {
          real_data[i] = creal(in_data[i]);
          imag_data[i] = cimag(in_data[i]);
        }
      }
    }
  }

  return mx_array;
}

static inline mxArray* bsp_matrix_to_matlab_struct(bsp_matrix_t* matrix) {
  const char* field_names[] = {
      "values", "indices_0", "indices_1", "pointers_to_1", "nrows",
      "ncols",  "nnz",       "is_iso",    "format",        "structure"};

  mxArray* mx_struct = mxCreateStructMatrix(1, 1, 10, field_names);

  mxSetField(mx_struct, 0, "values", bsp_array_to_matlab(&matrix->values));
  mxSetField(mx_struct, 0, "indices_0",
             bsp_array_to_matlab(&matrix->indices_0));
  mxSetField(mx_struct, 0, "indices_1",
             bsp_array_to_matlab(&matrix->indices_1));
  mxSetField(mx_struct, 0, "pointers_to_1",
             bsp_array_to_matlab(&matrix->pointers_to_1));

  mxSetField(mx_struct, 0, "nrows",
             mxCreateDoubleScalar((double) matrix->nrows));
  mxSetField(mx_struct, 0, "ncols",
             mxCreateDoubleScalar((double) matrix->ncols));
  mxSetField(mx_struct, 0, "nnz", mxCreateDoubleScalar((double) matrix->nnz));
  mxSetField(mx_struct, 0, "is_iso", mxCreateLogicalScalar(matrix->is_iso));

  mxSetField(mx_struct, 0, "format",
             mxCreateString(bsp_get_matrix_format_string(matrix->format)));
  mxSetField(mx_struct, 0, "structure",
             mxCreateString(bsp_get_structure_string(matrix->structure)));

  return mx_struct;
}

#endif
