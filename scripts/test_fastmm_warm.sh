# SPDX-License-Identifier: BSD-3-Clause

# Usage: ./test_fastmm.sh [/path/to/benchmark_read,/path/to/benchmark_write/] [/path/to/matrix_market/matrices/] [/path/to/binsparse/matrices] [optional: /path/to/filesystem/for/experiments]

BENCHMARK_BINARY=$1

SOURCE_DIR=$2

BINSPARSE_DIR=$3

EXPERIMENTS=$(basename ${SOURCE_DIR})

echo "Evaluating benchmark $(basename ${BENCHMARK_BINARY}) on ${EXPERIMENTS}"

if [ -z "$4" ]
then
  echo "No scratch directory provided for experiments, directly running experiments on source directory."
  EXPERIMENT_DIR=${SOURCE_DIR}
else
  SCRATCH_DIR=$4
  echo "Copying experimental data from ${SOURCE_DIR} to scratch directory ${SCRATCH_DIR}}."
  cp -r $SOURCE_DIR $SCRATCH_DIR
  EXPERIMENT_DIR=${SCRATCH_DIR}/${EXPERIMENTS}
fi

sync

echo "Benchmarking Binsparse using ${EXPERIMENT_DIR}"

for file in $(find ${EXPERIMENT_DIR} -iname "*.mtx")
do
  dataset=`echo ${file} | sed -E "s/.+\/(.+\/.+)\.mtx/\1/"`
  binsparse_file=${BINSPARSE_DIR}/${dataset}.coo.bsp.h5
  echo "${dataset} ${file} ${binsparse_file}"
  $BENCHMARK_BINARY $file ${binsparse_file} 6
done

if [ -z "$4" ]
then
  :
else
  echo "Deleting scratch directory \"${SCRATCH_DIR}/${EXPERIMENTS}\"."
  rm -r $SCRATCH_DIR/$EXPERIMENTS
fi
