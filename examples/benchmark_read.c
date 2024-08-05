#include <binsparse/binsparse.h>
#include <stdlib.h>
#include <time.h>

double gettime() {
  struct timespec time;
  clock_gettime(CLOCK_MONOTONIC, &time);
  return ((double) time.tv_sec) + ((double) 1e-9) * time.tv_nsec;
}

int compar(const void* a, const void* b) {
  double x = *((const double*) a);
  double y = *((const double*) b);

  double diff = x - y;

  if (diff > 0) {
    return 1;
  } else if (diff < 0) {
    return -1;
  } else {
    return 0;
  }
}

double compute_variance(double* x, size_t n) {
  double sum = 0;

  for (size_t i = 0; i < n; i++) {
    sum += x[i];
  }

  double mean = sum / n;

  double sum_of_squares = 0;
  for (size_t i = 0; i < n; i++) {
    sum_of_squares += (x[i] - mean) * (x[i] - mean);
  }

  return sum_of_squares / (n - 1);
}

void flush_cache() {
#ifdef __APPLE__
  system("bash -c \"sync && sudo purge\"");
#elif __linux__
  system("bash -c \"sync\" && sudo sh -c \"/usr/bin/echo 3 > "
         "/proc/sys/vm/drop_caches\"");
#else
  static_assert(false);
#endif
  usleep(100000);
}

int main(int argc, char** argv) {
  if (argc < 2) {
    fprintf(stderr, "usage: ./benchmark_read [file_name.h5]\n");
    return 1;
  }

  char* file_name = argv[1];

  printf("Opening %s\n", file_name);

  const int num_trials = 1;

  double durations[num_trials];

  size_t nbytes = 0;

  // To flush the filesystem cache before each trial, change to `true`.
  bool cold_cache = true;

  // If running warm cache experiments, read once to warm cache.
  if (!cold_cache) {
    bsp_matrix_t mat = bsp_read_matrix(file_name, NULL);
    bsp_destroy_matrix_t(mat);
  }

  for (size_t i = 0; i < num_trials; i++) {
    if (cold_cache) {
      flush_cache();
    }
    double begin = gettime();
    bsp_matrix_t mat = bsp_read_matrix(file_name, NULL);
    double end = gettime();
    durations[i] = end - begin;
    nbytes = bsp_matrix_nbytes(mat);
    bsp_destroy_matrix_t(mat);
  }

  printf("[");
  for (size_t i = 0; i < num_trials; i++) {
    printf("%lf", durations[i]);
    if (i + 1 < num_trials) {
      printf(", ");
    }
  }
  printf("]\n");

  qsort(durations, num_trials, sizeof(double), compar);

  double variance = compute_variance(durations, num_trials);

  double median_time = durations[num_trials / 2];

  printf("Read file in %lf seconds\n", median_time);

  if (num_trials > 1) {
    printf("Variance is %lf seconds, standard devication is %lf seconds\n",
           variance, sqrt(variance));
  }

  double gbytes = ((double) nbytes) / 1024 / 1024 / 1024;
  double gbytes_s = gbytes / median_time;

  printf("Achieved %lf GiB/s\n", gbytes_s);

  printf("[");
  for (size_t i = 0; i < num_trials; i++) {
    printf("%lf", durations[i]);
    if (i + 1 < num_trials) {
      printf(", ");
    }
  }
  printf("]\n");

  printf("FORPARSER: %s,%lf,%lf\n", file_name, median_time, gbytes_s);

  return 0;
}
