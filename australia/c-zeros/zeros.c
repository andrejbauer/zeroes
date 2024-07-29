#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h> // for sysconf
#include "compute.h"
#include "image.h"

int main(int argc, char* argv[]) {
    const char* usage = "Usage: zeros <xmin> <xmax> <ymin> <ymax> <xres> <coeff> ... <coeff> - <degree> ... <degree>\n";

    if (argc < 8) {
        fprintf(stderr, "%s", usage);
        exit(EXIT_FAILURE);
    }

    const double xmin = atof(argv[1]);
    const double xmax = atof(argv[2]);
    const double ymin = atof(argv[3]);
    const double ymax = atof(argv[4]);
    const int xres = atoi(argv[5]);
    const int yres = (int)((xres * (ymax-ymin)) / (xmax-xmin));

    int ci_max = 0;
    while (6 + ci_max < argc && (argv[6+ci_max][0] != '-' || argv[6+ci_max][1] != 0)) { ci_max++; }
    if (ci_max == 0) {
        fprintf(stderr, "%s\nError: specify at least one coefficient.", usage);
        exit(EXIT_FAILURE);
    }
    double coeff[ci_max];
    for (int ci = 0; ci < ci_max; ci++) { coeff[ci] = atof(argv[6+ci]); }

    const int di_max = argc - 7 - ci_max;
    if (di_max <= 0) {
        fprintf(stderr, "%s\nError: specify at least one degree.", usage);
        exit(EXIT_FAILURE);
    }
    int degree[di_max];
    for (int di = 0; di < di_max; di++) { degree[di] = atoi(argv[7+ci_max+di]); }

    unsigned int max_count = 0;
    pthread_mutex_t max_count_mutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_t image_mutex = PTHREAD_MUTEX_INITIALIZER;

    int num_threads = sysconf(_SC_NPROCESSORS_ONLN) - 1;
    fprintf(stderr, "Using %d threads\n", num_threads);
    pthread_t threads[num_threads];
    thread_data_t thread_data[num_threads];

    unsigned int *shared_image = (unsigned int *)calloc(sizeof(unsigned int), yres * xres);

    // Create work queue
    work_queue queue = { NULL, PTHREAD_MUTEX_INITIALIZER };
    int fixed_coeff[NUM_FIXED_COEFF];
    for (int di = 0; di < di_max; di++) {
        recursive_enqueue(&queue, fixed_coeff, 0, ci_max, degree[di]);
    }

    for (int t = 0; t < num_threads; t++) {
        thread_data[t].thread_id = t;
        thread_data[t].coeff = coeff;
        thread_data[t].ci_max = ci_max;
        thread_data[t].queue = &queue;
        thread_data[t].xmin = xmin;
        thread_data[t].xmax = xmax;
        thread_data[t].ymin = ymin;
        thread_data[t].ymax = ymax;
        thread_data[t].xres = xres;
        thread_data[t].yres = yres;
        thread_data[t].shared_image = shared_image;
        thread_data[t].image_mutex = &image_mutex;
        thread_data[t].max_count = &max_count;
        thread_data[t].max_count_mutex = &max_count_mutex;

        pthread_create(&threads[t], NULL, compute_zeroes, (void *)&thread_data[t]);
    }

    for (int t = 0; t < num_threads; t++) {
        pthread_join(threads[t], NULL);
    }

    save_tiff_image("output.tiff", shared_image, xres, yres);

    free(shared_image);
    return 0;
}
