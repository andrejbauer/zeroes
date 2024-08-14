#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include "compute.h"
#include "image.h"

typedef struct {
    double xmin, xmax, ymin, ymax;
    int xres;
    double *coeffs;
    int coeff_count;
    int *degrees;
    int degree_count;
} input_params_t;

int read_input_file(const char *filename, input_params_t *params) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Error opening input file");
        return -1;
    }

    char line[256];
    int got_xmin = 0, got_xmax = 0, got_ymin = 0, got_ymax = 0, got_xres = 0;

    while (fgets(line, sizeof(line), file)) {
        if (line[0] == '\n' || line[0] == '\r') {
            continue; // Ignore empty lines
        }

        char key[256];
        char value[256];
        if (sscanf(line, "%s %[^\n]", key, value) != 2) {  // Using %[^\n] to capture the rest of the line as value
            fprintf(stderr, "Error parsing line: %s\n", line);
            fclose(file);
            return -1;
        }

        if (strcmp(key, "xmin") == 0) {
            params->xmin = atof(value);
            got_xmin = 1;
        } else if (strcmp(key, "xmax") == 0) {
            params->xmax = atof(value);
            got_xmax = 1;
        } else if (strcmp(key, "ymin") == 0) {
            params->ymin = atof(value);
            got_ymin = 1;
        } else if (strcmp(key, "ymax") == 0) {
            params->ymax = atof(value);
            got_ymax = 1;
        } else if (strcmp(key, "xres") == 0) {
            params->xres = atoi(value);
            got_xres = 1;
        } else if (strcmp(key, "coeffs") == 0) {
            char *token = strtok(value, " ");
            params->coeff_count = 0;
            while (token) {
                params->coeffs = realloc(params->coeffs, (params->coeff_count + 1) * sizeof(double));
                if (!params->coeffs) {
                    perror("Error reallocating memory for coefficients");
                    fclose(file);
                    return -1;
                }
                params->coeffs[params->coeff_count++] = atof(token);
                token = strtok(NULL, " ");
            }
        } else if (strcmp(key, "degrees") == 0) {
            char *token = strtok(value, " ");
            params->degree_count = 0;
            while (token) {
                params->degrees = realloc(params->degrees, (params->degree_count + 1) * sizeof(int));
                if (!params->degrees) {
                    perror("Error reallocating memory for degrees");
                    fclose(file);
                    return -1;
                }
                params->degrees[params->degree_count++] = atoi(token);
                token = strtok(NULL, " ");
            }
        } else {
            fprintf(stderr, "Unknown key: %s\n", key);
            fclose(file);
            return -1;
        }
    }

    if (!got_xmin || !got_xmax || !got_ymin || !got_ymax || !got_xres || params->coeff_count == 0 || params->degree_count == 0) {
        fprintf(stderr, "Error: Missing required input parameters.\n");
        fclose(file);
        return -1;
    }

    fclose(file);

    // Print parsed parameters for debugging
    fprintf(stderr, "Parsed parameters:\n");
    fprintf(stderr, "xmin = %f\n", params->xmin);
    fprintf(stderr, "xmax = %f\n", params->xmax);
    fprintf(stderr, "ymin = %f\n", params->ymin);
    fprintf(stderr, "ymax = %f\n", params->ymax);
    fprintf(stderr, "xres = %d\n", params->xres);
    fprintf(stderr, "coeffs = ");
    for (int i = 0; i < params->coeff_count; i++) {
        fprintf(stderr, "%f ", params->coeffs[i]);
    }
    fprintf(stderr, "\ndegrees = ");
    for (int i = 0; i < params->degree_count; i++) {
        fprintf(stderr, "%d ", params->degrees[i]);
    }
    fprintf(stderr, "\n");

    return 0;
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <input_file> <output_file>\n", argv[0]);
        return EXIT_FAILURE;
    }

    initialize_gsl();

    const char *input_file = argv[1];
    const char *output_file = argv[2];

    input_params_t params = {0};
    if (read_input_file(input_file, &params) != 0) {
        return EXIT_FAILURE;
    }

    const int yres = (int)((params.xres * (params.ymax - params.ymin)) / (params.xmax - params.xmin));

    unsigned int max_count = 0;
    pthread_mutex_t max_count_mutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_t image_mutex = PTHREAD_MUTEX_INITIALIZER;

    int num_threads = sysconf(_SC_NPROCESSORS_ONLN) - 1;
    fprintf(stderr, "Using %d threads\n", num_threads);
    pthread_t threads[num_threads];
    thread_data_t thread_data[num_threads];

    unsigned int *shared_image = (unsigned int *)calloc(sizeof(unsigned int), yres * params.xres);
    if (!shared_image) {
        perror("Error allocating memory for image");
        return EXIT_FAILURE;
    }

    // Create work queue
    work_queue queue = { NULL, PTHREAD_MUTEX_INITIALIZER };
    int fixed_coeff[NUM_FIXED_COEFF];
    for (int di = 0; di < params.degree_count; di++) {
        recursive_enqueue(&queue, fixed_coeff, 0, params.coeff_count, params.degrees[di]);
    }

    for (int t = 0; t < num_threads; t++) {
        thread_data[t].thread_id = t;
        thread_data[t].coeff = params.coeffs;
        thread_data[t].ci_max = params.coeff_count;
        thread_data[t].queue = &queue;
        thread_data[t].xmin = params.xmin;
        thread_data[t].xmax = params.xmax;
        thread_data[t].ymin = params.ymin;
        thread_data[t].ymax = params.ymax;
        thread_data[t].xres = params.xres;
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

    save_tiff_image(output_file, shared_image, params.xres, yres);

    free(shared_image);
    free(params.coeffs);
    free(params.degrees);
    return 0;
}
