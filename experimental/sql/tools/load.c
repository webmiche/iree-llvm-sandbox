#include "tsc_x86.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define NR 32
#define CYCLES_REQUIRED 1e10
#define REP 1

extern void query(int64_t, int64_t *, int64_t *, int64_t *, int64_t *);

int benchmark(int64_t scale_factor) {
  int64_t row_count;
  FILE *fp;
  char *line = malloc(100);
  size_t len = 0;
  ssize_t read;

  switch (scale_factor) {
  case 1:
    row_count = 6001215;
    break;
  case 2:
    row_count = 11997996;
    break;
  case 3:
    row_count = 17996609;
    break;
  case 4:
    row_count = 23996604;
    break;
  case 5:
    row_count = 29999795;
    break;
  case 6:
    row_count = 36000148;
    break;
  case 7:
    row_count = 41995307;
    break;
  case 8:
    row_count = 47989007;
    break;
  case 9:
    row_count = 53986608;
    break;
  case 10:
    row_count = 59986052;
    break;
  case 16:
    row_count = 95988640;
    break;
  }
  char filename[48];
  snprintf(filename, 48, "/home/michel/MasterThesis/dbgen/lineitem_%ld.tbl",
           scale_factor);
  fp = fopen(filename, "r");
  if (fp == NULL)
    exit(EXIT_FAILURE);
  char delim = '|';

  int64_t *shipdate = (int64_t *)malloc(row_count * sizeof(int64_t));
  int64_t *discount = (int64_t *)malloc(row_count * sizeof(int64_t));
  int64_t *quantity = (int64_t *)malloc(row_count * sizeof(int64_t));
  int64_t *extendedprice = (int64_t *)malloc(row_count * sizeof(int64_t));
  for (int k = 0; k < row_count; k++) {
    getline(&line, &len, fp);
    char *token = strtok(line, &delim);
    char *end;
    for (int i = 0; i < 16; i++) {
      if (i == 4) {
        quantity[k] = strtol(token, &end, 10);
      }
      if (i == 5) {
        extendedprice[k] = strtol(token, &end, 10);
      }
      if (i == 6) {
        discount[k] = strtol(token, &end, 10);
      }
      if (i == 10) {
        shipdate[k] = strtol(token, &end, 10);
      }
      token = strtok(0, &delim);
    }
  }

  double start;
  double end;
  // double multiplier = 1;
  double cycles = 0;
  int64_t num_runs = 100;
  // do {
  //   num_runs = num_runs * multiplier;
  //   start = start_tsc();
  //   for (size_t i = 0; i < num_runs; i++) {
  //       query(row_count, shipdate, discount, quantity, extendedprice);
  //   }
  //   end = stop_tsc(start);

  //   cycles = (double)end;
  //   multiplier = (CYCLES_REQUIRED) / (cycles);
  // } while(multiplier < 2);

  double total_cycles = 0;
  printf("%ld\n", scale_factor);
  for (size_t j = 0; j < REP; j++) {

    for (size_t i = 0; i < num_runs; ++i) {
      start = start_tsc();
      query(row_count, shipdate, discount, quantity, extendedprice);
      end = stop_tsc(start);
      printf("%lf\n", ((double)end) / 2.3e9);
    }

    cycles = ((double)end) / num_runs;
    total_cycles += cycles;
  }
  total_cycles /= REP;

  free(shipdate);
  free(discount);
  free(extendedprice);
  free(quantity);
  fclose(fp);
  if (line)
    free(line);
  return 0;
}

int main(void) {
  benchmark(1);
  benchmark(2);
  benchmark(4);
  benchmark(8);
  benchmark(16);
  return 0;
}
