# SPDX-License-Identifier: BSD-3-Clause

# Usage: ./test.sh [/path/to/benchmark_read,/path/to/benchmark_write/] [/path/to/binsparse/matrices/] [optional: /path/to/filesystem/for/experiments]

BENCHMARK_BINARY=$1

SOURCE_DIR=$2

EXPERIMENTS=$(basename ${SOURCE_DIR})

echo "Evaluating benchmark $(basename ${BENCHMARK_BINARY}) on ${EXPERIMENTS}"

if [ -z "$3" ]
then
  echo "No scratch directory provided for experiments, directly running experiments on source directory."
  EXPERIMENT_DIR=${SOURCE_DIR}
else
  SCRATCH_DIR=$3
  echo "Copying experimental data from ${SOURCE_DIR} to scratch directory ${SCRATCH_DIR}}."
  cp -r $SOURCE_DIR $SCRATCH_DIR
  EXPERIMENT_DIR=${SCRATCH_DIR}/${EXPERIMENTS}
fi

sync

echo "Benchmarking Binsparse using ${EXPERIMENT_DIR}"

niterations=10

for (( i = 0; i < niterations; i++ ))
do
  for file in $(find ${EXPERIMENT_DIR} -iname "*.h5")
  do
    echo $file
    $BENCHMARK_BINARY $file
  done
done

if [ -z "$3" ]
then
  :
else
  echo "Deleting scratch directory \"${SCRATCH_DIR}/${EXPERIMENTS}\"."
  rm -r $SCRATCH_DIR/$EXPERIMENTS
fi
