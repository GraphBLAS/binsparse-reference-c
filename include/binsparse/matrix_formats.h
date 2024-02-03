#pragma once

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
  BSP_COOC = 20
} bsp_matrix_format_t;

char* bsp_get_matrix_format_string(bsp_matrix_format_t format) {
  if (format == BSP_DVEC) {
    return "DVEC";
  } else if (format == BSP_DMAT) {
    return "DMAT";
  } else if (format == BSP_DMATC) {
    return "DMATC";
  } else if (format == BSP_CVEC) {
    return "CVEC";
  } else if (format == BSP_CSR) {
    return "CSR";
  } else if (format == BSP_DCSR) {
    return "DCSR";
  } else if (format == BSP_DCSC) {
    return "DCSC";
  } else if (format == BSP_COO) {
    return "COO";
  } else if (format == BSP_COOC) {
    return "COOC";
  } else {
    return "";
  }
}
