#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "tsc_x86.h"

#define NR 32
#define CYCLES_REQUIRED 1e10
#define REP 50

extern void query(int64_t, int64_t*, int64_t*, int64_t*, int64_t*);

int main(void)
{
    FILE * fp;
    char* line = malloc(100);
    size_t len = 0;
    ssize_t read;

    fp = fopen("/home/michel/MasterThesis/dbgen/lineitem.tbl", "r");
    if (fp == NULL)
        exit(EXIT_FAILURE);
    char delim = '|';

    int64_t row_count = 6001215;
    int64_t* shipdate = (int64_t*) malloc(row_count * sizeof(int64_t));
    int64_t* discount = (int64_t*) malloc(row_count * sizeof(int64_t));
    int64_t* quantity = (int64_t*) malloc(row_count * sizeof(int64_t));
    int64_t* extendedprice = (int64_t*) malloc(row_count * sizeof(int64_t));
    for(int k = 0; k < row_count; k++) {
      getline(&line, &len, fp);
      char *token = strtok(line, &delim);
      char *end;
      for(int i = 0; i < 16; i++) {
        if(i == 4) {
          quantity[k] = strtol(token, &end, 10);
        }
        if(i == 5) {
          extendedprice[k] = strtol(token, &end, 10);
        }
        if(i == 6) {
          discount[k] = strtol(token, &end, 10);
        }
        if(i == 10) {
          shipdate[k] = strtol(token, &end, 10);
        }
        token = strtok(0, &delim);
      }
    }

    double start;
    double end;
    double multiplier = 1;
    double cycles = 0;
    int64_t num_runs = 100;
    do {
      num_runs = num_runs * multiplier;
      start = start_tsc();
      for (size_t i = 0; i < num_runs; i++) {
          query(row_count, shipdate, discount, quantity, extendedprice);
      }
      end = stop_tsc(start);
      printf("%lf\n", end);
      printf("%ld\n", num_runs);
      printf("%lf\n", multiplier);

      cycles = (double)end;
      multiplier = (CYCLES_REQUIRED) / (cycles);
    } while(multiplier < 2);


    double total_cycles = 0;
    for (size_t j = 0; j < REP; j++) {

        start = start_tsc();
        for (size_t i = 0; i < num_runs; ++i) {
          query(row_count, shipdate, discount, quantity, extendedprice);
        }
        end = stop_tsc(start);

        cycles = ((double)end) / num_runs;
        total_cycles += cycles;

    }
    total_cycles /= REP;

    printf("%lf\n", total_cycles/2.3e9);


    free(shipdate);
    free(discount);
    free(extendedprice);
    free(quantity);
    fclose(fp);
    if (line)
        free(line);
    exit(EXIT_SUCCESS);
}
