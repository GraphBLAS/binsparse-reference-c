# SPDX-FileCopyrightText: 2024 Binsparse Developers
#
# SPDX-License-Identifier: BSD-3-Clause

# Usage: ./test.sh [/path/to/benchmark_read,/path/to/benchmark_write/] [/path/to/binsparse/matrices/] [/path/to/filesystem/for/experiments] [compression level: [0-9] ...]

BENCHMARK_BINARY=$1

SOURCE_DIR=$2

EXPERIMENTS=$(basename ${SOURCE_DIR})

SCRATCH_DIR=$3

COMPRESSION_LEVEL=$4

EXPERIMENT_DIR=${SOURCE_DIR}

echo "Evaluating benchmark $(basename ${BENCHMARK_BINARY}) on ${EXPERIMENTS}"

sync

echo "Using scratch space ${SCRATCH_DIR}"

for file in $(find ${EXPERIMENT_DIR} -iname "*.h5")
do
  echo $file
  echo "${BENCHMARK_BINARY} ${file} ${SCRATCH_DIR} ${COMPRESSION_LEVEL}"
  ${BENCHMARK_BINARY} ${file} ${SCRATCH_DIR} ${COMPRESSION_LEVEL}
done
