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
#include <signal.h>
#include <getopt.h>

#include "find_min_max.h"
#include "utils.h"

int *child_pids = NULL;
int pnum;

void kill_children(int signum) {
    for (int i = 0; i < pnum; i++) {
        if (child_pids[i] > 0) {
            kill(child_pids[i], SIGKILL);
        }
    }
}

int main(int argc, char **argv) {
    int seed = -1;
    int array_size = -1;
    bool with_files = false;
    int timeout = 0;  // Таймаут по умолчанию

    while (true) {
        int current_optind = optind ? optind : 1;

        static struct option options[] = {{"seed", required_argument, 0, 0},
                                          {"array_size", required_argument, 0, 0},
                                          {"pnum", required_argument, 0, 0},
                                          {"by_files", no_argument, 0, 'f'},
                                          {"timeout", required_argument, 0, 0},
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
                        if (pnum <= 0) {
                            printf("pnum is a positive number\n");
                            return 1;
                        }
                        break;
                    case 3:
                        with_files = true;
                        break;
                    case 4:
                        timeout = atoi(optarg);
                        if (timeout <= 0) {
                            printf("timeout should be a positive number\n");
                            return 1;
                        }
                        break;
                    default:
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
        printf("Usage: %s --seed \"num\" --array_size \"num\" --pnum \"num\" [--timeout \"num\"]\n",
               argv[0]);
        return 1;
    }

    int *array = malloc(sizeof(int) * array_size);
    GenerateArray(array, array_size, seed);
    int active_child_processes = 0;

    child_pids = malloc(sizeof(int) * pnum);

    struct timeval start_time;
    gettimeofday(&start_time, NULL);

    int fd[2][2];  // file descriptors for pipes

    for (int i = 0; i < pnum; i++) {
        if (!with_files) {
            if (pipe(fd[i]) < 0) {
                printf("Pipe failed!\n");
                return 1;
            }
        }

        pid_t child_pid = fork();
        if (child_pid >= 0) {
            // successful fork
            child_pids[i] = child_pid;
            active_child_processes += 1;
            if (child_pid == 0) {
                // child process
                struct MinMax min_max;
                int start = i * array_size / pnum;
                int end = (i == pnum - 1) ? array_size : (i + 1) * array_size / pnum;

                min_max = GetMinMax(array, start, end);

                if (with_files) {
                    char filename[256];
                    sprintf(filename, "min_max_file_%d.txt", i);
                    FILE *fp = fopen(filename, "w");
                    if (fp == NULL) {
                        printf("Can't open file for writing\n");
                        return 1;
                    }
                    fprintf(fp, "%d %d", min_max.min, min_max.max);
                    fclose(fp);
                } else {
                    close(fd[i][0]);  // Close unused read end
                    write(fd[i][1], &min_max, sizeof(struct MinMax));
                    close(fd[i][1]);
                }
                return 0;
            }
        } else {
            printf("Fork failed!\n");
            return 1;
        }
    }

    // Устанавливаем обработчик для SIGALRM
    if (timeout > 0) {
        signal(SIGALRM, kill_children);
        alarm(timeout);  // Устанавливаем таймер на заданное количество секунд
    }

    while (active_child_processes > 0) {
        int status;
        pid_t finished_pid = waitpid(-1, &status, WNOHANG);  // Неблокирующий wait
        if (finished_pid > 0) {
            active_child_processes--;
        }
    }

    struct MinMax min_max;
    min_max.min = INT_MAX;
    min_max.max = INT_MIN;

    for (int i = 0; i < pnum; i++) {
        int min = INT_MAX;
        int max = INT_MIN;

        if (with_files) {
            char filename[256];
            sprintf(filename, "min_max_file_%d.txt", i);
            FILE *fp = fopen(filename, "r");
            if (fp == NULL) {
                printf("Can't open file for reading\n");
                return 1;
            }
            fscanf(fp, "%d %d", &min, &max);
            fclose(fp);
            remove(filename);  // Delete the temporary file
        } else {
            struct MinMax temp_min_max;
            close(fd[i][1]);  // Close unused write end
            read(fd[i][0], &temp_min_max, sizeof(struct MinMax));
            close(fd[i][0]);
            min = temp_min_max.min;
            max = temp_min_max.max;
        }

        if (min < min_max.min) min_max.min = min;
        if (max > min_max.max) min_max.max = max;
    }

    struct timeval finish_time;
    gettimeofday(&finish_time, NULL);

    double elapsed_time = (finish_time.tv_sec - start_time.tv_sec) * 1000.0;
    elapsed_time += (finish_time.tv_usec - start_time.tv_usec) / 1000.0;

    free(array);
    free(child_pids);

    printf("Min: %d\n", min_max.min);
    printf("Max: %d\n", min_max.max);
    printf("Elapsed time: %fms\n", elapsed_time);
    fflush(NULL);
    return 0;
}
