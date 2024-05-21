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
#else
  static_assert(false);
#endif
}

void flush_writes() {
#ifdef __APPLE__
  system("bash -c \"sync\"");
#else
  static_assert(false);
#endif
}

void delete_file(char* file_name) {
  char command[2048];
  snprintf(command, 2047, "rm %s", file_name);
  system(command);
}

int main(int argc, char** argv) {
  if (argc < 2) {
    fprintf(stderr, "usage: ./benchmark_read [file_name.h5] [optional: "
                    "compression_level]\n");
    return 1;
  }

  char* file_name = argv[1];

  int compression_level = 0;

  if (argc >= 3) {
    compression_level = atoi(argv[2]);
  }

  printf("Opening %s\n", file_name);

  const int num_trials = 10;

  double durations[num_trials];

  bsp_matrix_t mat = bsp_read_matrix(file_name, NULL);
  size_t nbytes = bsp_matrix_nbytes(mat);

  char output_filename[2048];
  strncpy(output_filename, "benchmark_write_file_n.h5", 2047);

  for (size_t i = 0; i < num_trials; i++) {
    flush_cache();
    output_filename[21] = '0' + i;
    printf("Writing to file %s\n", output_filename);

    double begin = gettime();
    bsp_write_matrix(output_filename, mat, NULL, NULL, compression_level);
    flush_writes();
    double end = gettime();
    durations[i] = end - begin;
    delete_file(output_filename);
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

  printf("Wrote file in %lf seconds\n", durations[num_trials / 2]);

  printf("Variance is %lf seconds, standard devication is %lf seconds\n",
         variance, sqrt(variance));

  double gbytes = ((double) nbytes) / 1024 / 1024 / 1024;
  double gbytes_s = gbytes / durations[num_trials / 2];

  printf("Achieved %lf GiB/s\n", gbytes_s);

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
