# Zeroes

This repository contained the code for computing images and animations of zeros of polynomiasl that were shown in my public talks:

* [Ničle](https://youtu.be/XtaRkV7PWTA?si=R_Rja7sS_ne9VZKM), TEDxUL, October 2014, see also the related [blog post](https://math.andrej.com/2014/10/16/tedx-zeroes/).
* [Zeros](https://mathematical-research-institute.sydney.edu.au/event/zeros-andrejbauer/), Sydney Mathematical Research Institute, August 13, 2024

## Prerequisites

The code works on MacOS and relies on the following software:

* [Python 3](https://www.python.org)
* XCode C compiler, although `gcc` ought to work
* [ImageMagick](https://imagemagick.org)
* [ffmpeg](https://ffmpeg.org)

XCode is part of Apple's ecosystem, and you can get the rest with the [Homebrew](https://brew.sh) package manager.

You will additionally need the following Python libraries:

* [pycairo](https://pypi.org/project/pycairo/)
* [pillow](https://pypi.org/project/pillow/)
* [numpy](https://pypi.org/project/numpy/)

They can be install with the Python `pip3` package manager.

To compile the C code, you need these libraries:

* [libtiff](https://libtiff.gitlab.io/libtiff/)
* [GNU Scientific Library](https://www.gnu.org/software/gsl/)

They can be installed with Homebrew by running `brew install libtiff gsl`.

## Programs
