#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <gsl/gsl_poly.h>
#include <gsl/gsl_errno.h>
#include <pthread.h>
#include "compute.h"

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
        double *poly = (double *)calloc(d + 1, sizeof(double));
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
            if (poly[d] > epsilon || poly[d] < -epsilon) {
                double z[2*d];
                int status = gsl_poly_complex_solve(poly, d+1, w, z);
                if (status == 0) {
                    /* pthread_mutex_lock(data->image_mutex); */
                    for (int i = 0; i < d; i++) {
                        if (-epsilon < z[2*i+1] && z[2*i+1] < epsilon && (z[2*i] < -0.5 || z[2*i] > 0.5)) {
                            continue;
                        }
                        int x = (int)((data->xres * (z[2*i] - data->xmin)) / (data->xmax - data->xmin));
                        int y = data->yres - (int)((data->yres * (z[2*i+1] - data->ymin)) / (data->ymax - data->ymin));
                        if (0 <= x && x < data->xres && 0 <= y && y < data->yres) {
                            unsigned int c = ++data->shared_image[data->xres * y + x];
                            if (max_count_local < c) { max_count_local = c; }
                        }
                    }
                    /* pthread_mutex_unlock(data->image_mutex); */
                }
            }
            for (j = NUM_FIXED_COEFF; j <= d && counter[j] == data->ci_max-1; j++) { counter[j] = 0; poly[j] = data->coeff[0]; }
            if (j <= d) { counter[j]++; poly[j] = data->coeff[counter[j]]; }
        } while (j <= d);
        gsl_poly_complex_workspace_free(w);
        /* fprintf(stderr, "Thread %d: Completed degree %d with fixed coeff indices", data->thread_id, d); */
        /* for (int i = 0; i < NUM_FIXED_COEFF; i++) { */
        /*     fprintf(stderr, " %d", task->fixed_coeff[i]); */
        /* } */
        /* fprintf(stderr, "\n"); */
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
