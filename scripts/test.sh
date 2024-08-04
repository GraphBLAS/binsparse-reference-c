
# Usage: ./test.sh [/path/to/benchmark_read,/path/to/benchmark_write/] [/path/to/binsparse/matrices/] [/path/to/filesystem/for/experiments]

BENCHMARK_BINARY=$1

SOURCE_DIR=$2

SCRATCH_DIR=$3

EXPERIMENTS=$(basename ${SOURCE_DIR})

cp -r $SOURCE_DIR $SCRATCH_DIR
sync
echo "Benchmarking Binsparse using \"${SCRATCH_DIR}/${EXPERIMENTS}\""

niterations=10

for (( i = 0; i < niterations; i++ ))
do
  for file in $(find ${SCRATCH_DIR}/${EXPERIMENTS} -iname "*.h5")
  do
    echo $file
    $BENCHMARK_BINARY $file
  done
done

echo "Deleting directory \"${SCRATCH_DIR}/${EXPERIMENTS}\""
rm -r $SCRATCH_DIR/$EXPERIMENTS
