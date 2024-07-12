#pragma once

#include <string.h>

typedef enum bsp_matrix_format_t {
  BSP_DVEC = 11,
  BSP_DMAT = 12,
  BSP_DMATR = 12,
  BSP_DMATC = 13,
  BSP_CVEC = 14,
  BSP_CSR = 15,
  BSP_CSC = 16,
  BSP_DCSR = 17,
  BSP_DCSC = 18,
  BSP_COO = 19,
  BSP_COOR = 19,
  BSP_COOC = 20,
  BSP_INVALID_FORMAT = 21
} bsp_matrix_format_t;

char* bsp_get_matrix_format_string(bsp_matrix_format_t format) {
  if (format == BSP_DVEC) {
    return (char*) "DVEC";
  } else if (format == BSP_DMAT) {
    return (char*) "DMAT";
  } else if (format == BSP_DMATC) {
    return (char*) "DMATC";
  } else if (format == BSP_CVEC) {
    return (char*) "CVEC";
  } else if (format == BSP_CSR) {
    return (char*) "CSR";
  } else if (format == BSP_DCSR) {
    return (char*) "DCSR";
  } else if (format == BSP_DCSC) {
    return (char*) "DCSC";
  } else if (format == BSP_COO) {
    return (char*) "COO";
  } else if (format == BSP_COOC) {
    return (char*) "COOC";
  } else {
    return (char*) "";
  }
}

bsp_matrix_format_t bsp_get_matrix_format(char* format) {
  if (strcmp("DVEC", format) == 0) {
    return BSP_DVEC;
  } else if (strcmp("DMAT", format) == 0) {
    return BSP_DMAT;
  } else if (strcmp("DMATR", format) == 0) {
    return BSP_DMATR;
  } else if (strcmp("DMATC", format) == 0) {
    return BSP_DMATC;
  } else if (strcmp("CVEC", format) == 0) {
    return BSP_CVEC;
  } else if (strcmp("CSR", format) == 0) {
    return BSP_CSR;
  } else if (strcmp("CSC", format) == 0) {
    return BSP_CSC;
  } else if (strcmp("DCSR", format) == 0) {
    return BSP_DCSR;
  } else if (strcmp("DCSC", format) == 0) {
    return BSP_DCSC;
  } else if (strcmp("COO", format) == 0) {
    return BSP_COO;
  } else if (strcmp("COOR", format) == 0) {
    return BSP_COOR;
  } else if (strcmp("COOC", format) == 0) {
    return BSP_COOC;
  } else {
    return BSP_INVALID_FORMAT;
  }
}
