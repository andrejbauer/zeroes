#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <gsl/gsl_poly.h>
#include <gsl/gsl_errno.h>
#include <pthread.h>
#include <gsl/gsl_complex.h>
#include <gsl/gsl_complex_math.h>
#include "compute.h"

// Define the progress tracking variables
unsigned int completed_tasks = 0;
unsigned int total_tasks = 0;

pthread_mutex_t progress_mutex = PTHREAD_MUTEX_INITIALIZER;

void print_progress() {
    pthread_mutex_lock(&progress_mutex);
    unsigned int local_completed_tasks = completed_tasks;
    pthread_mutex_unlock(&progress_mutex);

    double progress = (double)local_completed_tasks / total_tasks * 100;
    fprintf(stderr, "Progress: %.2f%%\r", progress);
    fflush(stderr); // Flush the output to ensure it is printed immediately
}

void enqueue(work_queue *queue, int fixed_coeff[NUM_FIXED_COEFF], int degree) {
    work_item *item = (work_item *)malloc(sizeof(work_item));
    for (int i = 0; i < NUM_FIXED_COEFF; i++) {
        item->fixed_coeff[i] = fixed_coeff[i];
    }
    item->degree = degree;
    item->next = NULL;

    pthread_mutex_lock(&queue->mutex);
    item->next = queue->head;
    queue->head = item;
    total_tasks++;  // Increment the total number of tasks
    pthread_mutex_unlock(&queue->mutex);
}

work_item* dequeue(work_queue *queue) {
    pthread_mutex_lock(&queue->mutex);
    work_item *item = queue->head;
    if (item != NULL) {
        queue->head = item->next;
    }
    pthread_mutex_unlock(&queue->mutex);

    return item;
}

void recursive_enqueue(work_queue *queue, int *fixed_coeff, int depth, int ci_max, int degree) {
    if (depth == NUM_FIXED_COEFF) {
        enqueue(queue, fixed_coeff, degree);
        return;
    }
    for (int i = 0; i < ci_max; i++) {
        fixed_coeff[depth] = i;
        recursive_enqueue(queue, fixed_coeff, depth + 1, ci_max, degree);
    }
}

void *compute_zeroes(void *thread_data) {
    thread_data_t *data = (thread_data_t *)thread_data;
    const double epsilon = 1.0e-20;
    unsigned int max_count_local = 0;

    while (1) {
        work_item *task = dequeue(data->queue);
        if (task == NULL) {
            break; // No more work items
        }

        int d = task->degree;
        gsl_complex *poly = (gsl_complex *)calloc(d + 1, sizeof(gsl_complex));
        int *counter = (int *)calloc(d + 1, sizeof(int));
        int j;

        // Initialize polynomial coefficients
        for (j = 0; j <= d; j++) {
            if (j < NUM_FIXED_COEFF) {
                poly[j] = data->coeff[task->fixed_coeff[j]];
            } else {
                poly[j] = data->coeff[0];
            }
            counter[j] = 0;
        }

        gsl_poly_complex_workspace *w = gsl_poly_complex_workspace_alloc(d+1);
        do {
            if (GSL_REAL(poly[d]) > epsilon || GSL_REAL(poly[d]) < -epsilon ||
                GSL_IMAG(poly[d]) > epsilon || GSL_IMAG(poly[d]) < -epsilon) {
                double z[2*d];
                int status = gsl_poly_complex_solve((double *)poly, d+1, w, z);
                if (status == 0) {
                    for (int i = 0; i < d; i++) {
                        if (-epsilon < z[2*i+1] && z[2*i+1] < epsilon && (z[2*i] < -0.5 || z[2*i] > 0.5)) {
                            continue;
                        }
                        int x = (int)((data->xres * (z[2*i] - data->xmin)) / (data->xmax - data->xmin));
                        int y = data->yres - (int)((data->yres * (z[2*i+1] - data->ymin)) / (data->ymax - data->ymin));
                        if (0 <= x && x < data->xres && 0 <= y && y < data->yres) {
                            pthread_mutex_lock(data->image_mutex);
                            unsigned int c = ++data->shared_image[data->xres * y + x];
                            pthread_mutex_unlock(data->image_mutex);
                            if (max_count_local < c) { max_count_local = c; }
                        }
                    }
                }
            }
            for (j = NUM_FIXED_COEFF; j <= d && counter[j] == data->ci_max-1; j++) { counter[j] = 0; poly[j] = data->coeff[0]; }
            if (j <= d) { counter[j]++; poly[j] = data->coeff[counter[j]]; }
        } while (j <= d);
        gsl_poly_complex_workspace_free(w);

        pthread_mutex_lock(&progress_mutex);
        completed_tasks++;
        pthread_mutex_unlock(&progress_mutex);
        if (completed_tasks % 100 == 0) { // Adjust this value as needed to control frequency of progress updates
            print_progress();
        }

        free(poly);
        free(counter);
        free(task);
    }

    pthread_mutex_lock(data->max_count_mutex);
    if (*data->max_count < max_count_local) {
        *data->max_count = max_count_local;
    }
    pthread_mutex_unlock(data->max_count_mutex);

    return NULL;
}
