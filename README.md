# Zeros

This repository contains code for computing images and animations of zeros of polynomials that were shown in my public talks:

* [Ničle](https://youtu.be/XtaRkV7PWTA?si=R_Rja7sS_ne9VZKM), TEDxUL, October 2014, see also the related [blog post](https://math.andrej.com/2014/10/16/tedx-zeroes/).
* [Zeros](https://mathematical-research-institute.sydney.edu.au/event/zeros-andrejbauer/), Sydney Mathematical Research Institute, August 13, 2024

## Prerequisites

The code works on MacOS, but should also run on Linux. It relies on the following software:

* [Python 3](https://www.python.org)
* XCode C compiler, although `gcc` ought to work
* [ImageMagick](https://imagemagick.org)
* [ffmpeg](https://ffmpeg.org)

XCode is part of Apple's ecosystem. There are many ways to obtain Python 3, just visit the web site. You can get the rest with the [Homebrew](https://brew.sh) package manager:

    brew install imagemagick ffmpeg

You will additionally need the following Python libraries:

* [pycairo](https://pypi.org/project/pycairo/)
* [pillow](https://pypi.org/project/pillow/)
* [numpy](https://pypi.org/project/numpy/)

They can be install with the Python `pip3` package manager:

    pip3 install pycairo pillow numpy

To compile the C code, you need these libraries:

* [libtiff](https://libtiff.gitlab.io/libtiff/)
* [GNU Scientific Library](https://www.gnu.org/software/gsl/)

They can be installed with Homebrew by running

    brew install libtiff gsl

## Programs

There are two programs:

1. `zeros` is a C program that computes zeros of polynomials with given coefficients and degrees, and generates grayscale TIFF, see the folder [`C`](./C)
2. `algebraic.py` is a Python program that computes zeros of polynommials with given complexity (sum of absolute values of coefficients) and degrees, and generates PNG images, see the folder [`algebraic`](./algebraic/).
