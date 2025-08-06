# SPDX-FileCopyrightText: 2024 Binsparse Developers
#
# SPDX-License-Identifier: BSD-3-Clause

target_sources(binsparse PRIVATE
  read_matrix.c
  src/read_tensor.c
  write_matrix.c
  src/write_tensor.c
)
