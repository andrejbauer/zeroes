#include <stdio.h>
#include <stdlib.h>
#include <tiffio.h>
#include "image.h"

void save_tiff_image(const char *filename, uint32_t *image_data, int width, int height) {
    TIFF *image;

    if ((image = TIFFOpen(filename, "w")) == NULL) {
        fprintf(stderr, "Could not open output file for writing\n");
        exit(1);
    }

    TIFFSetField(image, TIFFTAG_IMAGEWIDTH, width);
    TIFFSetField(image, TIFFTAG_IMAGELENGTH, height);
    TIFFSetField(image, TIFFTAG_SAMPLESPERPIXEL, 1);
    TIFFSetField(image, TIFFTAG_BITSPERSAMPLE, 32);
    TIFFSetField(image, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
    TIFFSetField(image, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
    TIFFSetField(image, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);

    for (int y = 0; y < height; y++) {
        if (TIFFWriteScanline(image, &image_data[y * width], y, 0) < 0) {
            fprintf(stderr, "Could not write image data\n");
            TIFFClose(image);
            exit(1);
        }
    }

    TIFFClose(image);
}
