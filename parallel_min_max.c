#include <ctype.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <getopt.h>

#include "find_min_max.h"
#include "utils.h"

int main(int argc, char **argv) {
  int seed = -1;
  int array_size = -1;
  int pnum = -1;
  bool with_files = false;

  while (true) {
    int current_optind = optind ? optind : 1;

    static struct option options[] = {{"seed", required_argument, 0, 0},
                                      {"array_size", required_argument, 0, 0},
                                      {"pnum", required_argument, 0, 0},
                                      {"by_files", no_argument, 0, 'f'},
                                      {0, 0, 0, 0}};

    int option_index = 0;
    int c = getopt_long(argc, argv, "f", options, &option_index);

    if (c == -1) break;

    switch (c) {
      case 0:
        switch (option_index) {
          case 0:
            seed = atoi(optarg);
            if (seed <= 0) {
                printf("seed is a positive number\n");
                return 1;
            }
            break;
          case 1:
            array_size = atoi(optarg);
            if (array_size <= 0) {
                printf("array_size is a positive number\n");
                return 1;
            }
            break;
          case 2:
            pnum = atoi(optarg);
            if(pnum <= 0){
                printf("PNUM: Size shoud be positive!");
                return 1;
            }
            break;
          case 3:
            with_files = true;
            break;

          defalut:
            printf("Index %d is out of options\n", option_index);
        }
        break;
      case 'f':
        with_files = true;
        break;

      case '?':
        break;

      default:
        printf("getopt returned character code 0%o?\n", c);
    }
  }

  if (optind < argc) {
    printf("Has at least one no option argument\n");
    return 1;
  }

  if (seed == -1 || array_size == -1 || pnum == -1) {
    printf("Usage: %s --seed \"num\" --array_size \"num\" --pnum \"num\" \n",
           argv[0]);
    return 1;
  }

  int *array = malloc(sizeof(int) * array_size);
  GenerateArray(array, array_size, seed);
  int active_child_processes = 0;

  struct timeval start_time;
  gettimeofday(&start_time, NULL);

  int block_size = array_size/pnum;
  int p1[2], p2[2];
  pipe(p1);
  pipe(p2);  


    for (int i = 0; i < pnum; i++) {
        pid_t child_pid = fork();
        if (child_pid >= 0) {
          // successful fork
          active_child_processes += 1;
          if (child_pid == 0) {
            // child process
            struct MinMax min_max = GetMinMax(array, block_size*i, block_size*(i+1));
        
            if (with_files) {
              char file_name[12];
              sprintf(file_name, "%d", i);
        
              FILE *fp = fopen(file_name, "w");
              fprintf(fp, "%d %d", min_max.min, min_max.max);
              fclose(fp);
            } else {
              write(p1[1], &min_max.min, sizeof(int));
              write(p2[1], &min_max.max, sizeof(int));
            }
            return 0;
          }
        
        } else {
          printf("Fork failed!\n");
          return 1;
        }
  }

  int status;
  while (active_child_processes > 0) {
    wait(&status);

    active_child_processes -= 1;
  }

  struct MinMax min_max;
  min_max.min = INT_MAX;
  min_max.max = INT_MIN;

  for (int i = 0; i < pnum; i++) {
    int min = INT_MAX;
    int max = INT_MIN;

    if (with_files) {
      char file_name[12];
      sprintf(file_name, "%d", i);
      
      FILE *fp = fopen(file_name, "r");
      fscanf(fp, "%d %d", &min, &max);
      fclose(fp);
      remove(file_name);
    } else {
      read(p1[0], &min, sizeof(int));
      read(p2[0], &max, sizeof(int));

      printf("locMin: %d\tlocMax: %d\n", min, max);
      
    }

    if (min < min_max.min) min_max.min = min;
    if (max > min_max.max) min_max.max = max;
  }

  close(p1[0]);
  close(p1[1]);
  close(p2[0]);
  close(p2[1]);

  struct timeval finish_time;
  gettimeofday(&finish_time, NULL);

  double elapsed_time = (finish_time.tv_sec - start_time.tv_sec) * 1000.0;
  elapsed_time += (finish_time.tv_usec - start_time.tv_usec) / 1000.0;

  free(array);

  printf("Min: %d\n", min_max.min);
  printf("Max: %d\n", min_max.max);
  printf("Elapsed time: %fms\n", elapsed_time);
  fflush(NULL);
  return 0;
}