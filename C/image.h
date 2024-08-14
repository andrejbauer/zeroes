#ifndef IMAGE_H
#define IMAGE_H

#include <stdint.h>

void save_tiff_image(const char *filename, uint32_t *image_data, int width, int height);

#endif // IMAGE_H
