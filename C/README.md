# Visualization of zeros of polynomals

This folder contains the C program for calculating zeros in parallel.

## Compilation

Compile it by running `make`. You make have to edit the `Makefile` if the compiler does not find the `libtiff` and `gsl` librariers.

## Usage

The C program is used as follows:

    ./zeroes <input-file> <output-file>

The input file has the following format:

```
xmin -2.0
xmax 2.0
ymin -2.0
ymax 2.0
xres 1024
coeffs -1.0 1.0
degrees 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17
```

The `xmin`, `xmax`, `ymin`, `ymax` values determine the region in the complex plane that should be displayed.
The horizontal resolution of the output image is `xres`, and the vertical resolution is compute accordingly.
The coefficients to be used are given by `coeffs` and the degrees by `degrees`. The above example will generate all polynomials with coefficients -1 and 1 of degrees up to 17.

The output file is a 32-bit grayscale TIFF image. The gray levels are log-scale frequency counts of how many times each pixel was hit by a zero. Thus whiter regions have a greater density of zeroes. In order to make a nice picture from the output we need to fiddle with the image colors. ImageMagick is a wonderful tool for doing precisely that.

For example, using the above parameters (also found in [`littlewood.txt`](./littlewood.txt)) we run the program as follows:

    ./zeros littlewood.txt picture.tiff

Then we process the resulting `picture.tiff` using ImageMagick:

    convert picture.tiff -evaluate log 400000 -normalize -fill orange -tint 100 picture.png

You can also experiment with `-contrast-stretch`, and for really good quality you will need
some competence with ImageMagick.

## Generating a movie

If you are planning to generate movies, you can generate a really big picture and then cut
out movie frames from it using ImageMagick. Then you can combine the frames into a movie
with `ffmpeg`. The [`movie.py`](./movie.py) program helps you generate a movie, as follows.

First we create a very large image and process it to give it some colors:

    ./zeros movie-example.txt big-picture.tiff
    convert big-picture.tiff -evaluate log 400000 -normalize -fill cyan -tint 100 big-picture.png

Then you look at [`movie.py`](./movie.py) and edit it to define whatever flyovers and zooms you are interested in, and also make sure that you use the correct image (sorry, it's not a very user-friendly program). Then, to generate a movie for one of the flyovers, say `zoom`, you run:

    ./movie.py zoom
