#ifndef COMPUTE_H
#define COMPUTE_H

#include <pthread.h>

#define NUM_FIXED_COEFF 8

typedef struct work_item {
    int fixed_coeff[NUM_FIXED_COEFF]; // Indices of the fixed coefficients for the combination
    int degree; // Polynomial degree
    struct work_item *next;
} work_item;

typedef struct {
    work_item *head;
    pthread_mutex_t mutex;
} work_queue;

typedef struct {
    int thread_id;
    const double *coeff;
    int ci_max;
    work_queue *queue;
    double xmin, xmax, ymin, ymax;
    int xres, yres;
    unsigned int *shared_image;
    pthread_mutex_t *image_mutex;
    unsigned int *max_count;
    pthread_mutex_t *max_count_mutex;
} thread_data_t;

void enqueue(work_queue *queue, int fixed_coeff[NUM_FIXED_COEFF], int degree);
work_item* dequeue(work_queue *queue);
void recursive_enqueue(work_queue *queue, int *fixed_coeff, int depth, int ci_max, int degree);
void *compute_zeroes(void *thread_data);

#endif // COMPUTE_H
