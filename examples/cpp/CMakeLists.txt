# SPDX-FileCopyrightText: 2024 Binsparse Developers
#
# SPDX-License-Identifier: BSD-3-Clause

function(add_example example_name)
  add_executable(${example_name}-cpp ${example_name}.cpp)
  target_link_libraries(${example_name}-cpp binsparse-rc)
endfunction()

add_example(simple_matrix_write)
add_example(simple_matrix_read)
add_example(simple_read)
add_example(simple_write)
add_example(mtx2bsp)
add_example(bsp2mtx)
add_example(check_equivalence)
add_example(bsp-ls)
add_example(tensor_test)
add_example(benchmark_read)
add_example(benchmark_write)
