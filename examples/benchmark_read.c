#include <binsparse/binsparse.h>
#include <time.h>

double gettime() {
  struct timespec time;
  clock_gettime(CLOCK_MONOTONIC, &time);
  return ((double) time.tv_sec) + ((double) 1e-9) * time.tv_nsec;
}

int compar(const void* a, const void* b) {
  double x = *((const double*) a);
  double y = *((const double*) b);

  return x - y;
}

int main(int argc, char** argv) {
  if (argc < 2) {
    fprintf(stderr, "usage: ./benchmark_read [file_name.h5]\n");
    return 1;
  }

  char* file_name = argv[1];

  printf("Opening %s\n", file_name);

  const int num_trials = 10;

  double durations[num_trials];

  for (size_t i = 0; i < num_trials; i++) {
    double begin = gettime();
    bsp_matrix_t mat = bsp_read_matrix(file_name, NULL);
    double end = gettime();
    durations[i] = end - begin;
    bsp_destroy_matrix_t(mat);
  }

  qsort(durations, num_trials, sizeof(double), compar);

  printf("Read file in %lf seconds\n", durations[num_trials / 2]);

  printf("[");
  for (size_t i = 0; i < num_trials; i++) {
    printf("%lf", durations[i]);
    if (i + 1 < num_trials) {
      printf(", ");
    }
  }
  printf("]\n");

  return 0;
}
