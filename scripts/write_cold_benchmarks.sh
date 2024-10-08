# SPDX-FileCopyrightText: 2024 Binsparse Developers
#
# SPDX-License-Identifier: BSD-3-Clause

# COO No Compression
./test_write.sh /home/xiii/src/binsparse-reference-c/build/examples/benchmark_write /media/xiii/zunyingdie/data/SuiteSparse_coo_noz_primary /media/xiii/zunyingdie/data/scratch 0 > br_coo_noz.out 2>&1

# COO GZip 1 Compression
./test_write.sh /home/xiii/src/binsparse-reference-c/build/examples/benchmark_write /media/xiii/zunyingdie/data/SuiteSparse_coo_gzip1_primary /media/xiii/zunyingdie/data/scratch 1 > br_coo_gz1.out 2>&1

# CSR No Compression
./test_write.sh /home/xiii/src/binsparse-reference-c/build/examples/benchmark_write /media/xiii/zunyingdie/data/SuiteSparse_csr_noz_primary /media/xiii/zunyingdie/data/scratch 0 > br_csr_noz.out 2>&1

# CSR GZip 1 Compression
./test_write.sh /home/xiii/src/binsparse-reference-c/build/examples/benchmark_write /media/xiii/zunyingdie/data/SuiteSparse_csr_gzip1_primary /media/xiii/zunyingdie/data/scratch 1 > br_csr_gz1.out 2>&1

# MTX No Compression
# TODO
# ./test_write_fastmm.sh /home/xiii/src/fast_mm_benchmarks/build/examples/benchmark_write /media/xiii/zunyingdie/data/SuiteSparse_MM_noz_primary /media/xiii/zunyingdie/data/SuiteSparse_coo_noz_primary > br_mtx_noz.out 2>&1
