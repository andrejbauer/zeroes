#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <gsl/gsl_poly.h>
#include <gsl/gsl_errno.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h> // for sysconf

// Define the number of fixed coefficients
#define NUM_FIXED_COEFF 6

// Define work_item and work_queue structures
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

// Function to add work items to the queue
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

// Function to remove and return work items from the queue
work_item* dequeue(work_queue *queue) {
    pthread_mutex_lock(&queue->mutex);
    work_item *item = queue->head;
    if (item != NULL) {
        queue->head = item->next;
    }
    pthread_mutex_unlock(&queue->mutex);

    return item;
}

// Recursively enqueue tasks
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

// Thread function to process the work items
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
                    pthread_mutex_lock(data->image_mutex);
                    for (int i = 0; i < d; i++) {
                        if (-epsilon < z[2*i+1] && z[2*i+1] < epsilon && (z[2*i] < -0.5 || z[2*i] > 0.5)) {
                            continue;
                        }
                        int x = (int)((data->xres * (z[2*i] - data->xmin)) / (data->xmax - data->xmin));
                        int y = data->yres - (int)((data->yres * (z[2*i+1] - data->ymin)) / (data->ymax - data->ymin));
                        if (0 <= x && x < data->xres && 0 <= y && y < data->yres) {
                            int c = ++data->shared_image[data->xres * y + x];
                            if (max_count_local < c) { max_count_local = c; }
                        }
                    }
                    pthread_mutex_unlock(data->image_mutex);
                }
            }
            // Update polynomial for next coefficient combination starting from the NUM_FIXED_COEFF coefficient
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

    printf("P6\n# Zeroes, xmin=%lf, xmax=%lf, ymin=%lf, ymax=%lf, max_count=%d, coeffs=[", xmin, xmax, ymin, ymax, max_count);
    for (int ci = 0; ci < ci_max; ci++) { printf("%lf%s", coeff[ci], (ci < ci_max-1 ? ", " : "],")); }
    printf("degrees = [");
    for (int di = 0; di < di_max; di++) { printf("%d%s", degree[di], (di < di_max-1 ? ", " : "],")); }
    printf("\n");
    printf("%d\n%d\n%d\n", xres, yres, 0xffff);

    double r = 65535.0 / log((double)max_count);
    for (int y = 0; y < yres; y++) {
        for (int x = 0; x < xres; x++) {
            unsigned char color[6];
            unsigned int k = (unsigned int)(r * log((float)shared_image[xres * y + x]));
            color[0] = k >> 8;
            color[1] = k & 255;
            color[2] = k >> 8;
            color[3] = k & 255;
            color[4] = k >> 8;
            color[5] = k & 255;
            fwrite(color, 6, 1, stdout);
        }
    }

    free(shared_image);
    return 0;
}
