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
