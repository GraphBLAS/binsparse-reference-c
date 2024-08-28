
# Usage: ./test_fastmm.sh [/path/to/benchmark_read,/path/to/benchmark_write/] [/path/to/binsparse/matrices/] [/path/to/filesystem/for/experiments]

BENCHMARK_BINARY=$1

SOURCE_DIR=$2

SCRATCH_DIR=$3

EXPERIMENTS=$(basename ${SOURCE_DIR})

echo "Evaluating benchmark $(basename ${BENCHMARK_BINARY}) on ${EXPERIMENTS}"

EXPERIMENT_DIR=${SOURCE_DIR}

sync

echo "Benchmarking Binsparse using ${EXPERIMENT_DIR}"

for file in $(find ${EXPERIMENT_DIR} -iname "*.h5")
do
  dataset=`echo ${file} | sed -E "s/.+\/(.+\/.+)\.h5/\1/"`
  $BENCHMARK_BINARY ${SCRATCH_DIR} $file
done

