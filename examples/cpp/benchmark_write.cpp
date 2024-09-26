// SPDX-FileCopyrightText: 2024 Binsparse Developers
//
// SPDX-License-Identifier: BSD-3-Clause

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
  system("bash -c \"sync\" && sudo echo 3 > /proc/sys/vm/drop_caches");
#else
  static_assert(false);
#endif
}

void flush_writes() {
#ifdef __APPLE__
  system("bash -c \"sync\"");
#elif __linux__
  system("bash -c \"sync\"");
#else
  static_assert(false);
#endif
}

void delete_file(const char* file_name) {
  char command[2048];
  snprintf(command, 2047, "rm %s", file_name);
  system(command);
}

int main(int argc, char** argv) {
  if (argc < 2) {
    fprintf(stderr,
            "usage: ./benchmark_read [file_name.h5] [scratch_space] [optional: "
            "compression_level]\n");
    return 1;
  }

  char* file_name = argv[1];
  char* scratch_space = argv[2];

  int compression_level = 0;

  if (argc >= 4) {
    compression_level = atoi(argv[3]);
  }

  printf("Opening %s\n", file_name);

  const int num_trials = 1;

  double durations[num_trials];

  bsp_matrix_t mat = bsp_read_matrix(file_name, NULL);
  size_t nbytes = bsp_matrix_nbytes(mat);

  char output_filename[2048];
  strncpy(output_filename, scratch_space, 2047);
  strncpy(output_filename + strlen(scratch_space), "/benchmark_write_file_n.h5",
          2047 - strlen(scratch_space));

  // Current output name logic does not do much.
  assert(num_trials <= 10);

  // To flush the filesystem cache before each trial, change to `true`.
  bool cold_cache = false;

  // To flush each write to the filesystem and include this in the timing,
  // change to `true`.
  bool flush_each_write = true;

  for (size_t i = 0; i < num_trials; i++) {
    if (cold_cache) {
      flush_cache();
    }

    output_filename[strlen(scratch_space) + 21] = '0' + i;
    printf("Writing to file %s\n", output_filename);

    double begin = gettime();
    bsp_write_matrix(output_filename, mat, NULL, NULL, compression_level);

    if (flush_each_write) {
      flush_writes();
    }

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

  double median_time = durations[num_trials / 2];

  printf("Wrote file in %lf seconds\n", median_time);

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
