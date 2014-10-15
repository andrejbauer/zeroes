# Zeroes of polynomials

Here is the code used to create the images from my [TEDx
talk](http://tedxul.si/speakers/andrej-bauer/), and some extras. It is all released under
the permissive [MIT license](http://opensource.org/licenses/MIT), so please have fun with
it and send me improvements (as `git` pull requests if possible).

The code and the TEDx talk were of course heavily inspired by John Baez's
[The Beauty of roots](http://www.math.ucr.edu/home/baez/roots/), and the work of people
mentioned therein.

Here is a short explanation of the relevant files:

* `zeroes.c`: a C program for computing & drawing the zeroes of polynomials
* `Makefile`: a make file to compile `zeroes.c`
* `zeroes.py`: Python program to draw the zeroes as disks
* `animate.py`: example of how to make an animation using `zeroes.py`
* `examples`: examples of images created by the above programs

The TEDx movies generated with these programs can be seen in my [Vimeo "Zeroes" album](https://vimeo.com/album/3086303), as well as some additional ones.

## Compilation

### Prerequisites

To compile `zeroes.c` you need the [GNU Scientific
Library](http://www.gnu.org/software/gsl/). It is available on most Linux distributions,
for instance on a Debian-based Linux you can get it with

    sudo apt-get install libgsl0-dev

On OSX you can get it via Homebrew with

    brew install gsl

For the Python files to work you need [numpy](http://www.numpy.org) and [Pillow](http://python-pillow.github.io) (which is a newer version of [PIL](http://www.pythonware.com/products/pil/)).

In order to do process the created images you will need
[ImageMagick](http://www.imagemagick.org) and [ffmpeg](http://ffmpeg.org). These are again
available through most Linux package managers, and through Homebrew on OSX.

I have no idea how to make this stuff work under Windows. If you do, please provide some instructions.

### Compilation

The Python code needs no compilation, of course.

Before you compile the C code uncomment the correct `LIBS` definition in the `Makefile`, then type `make`.


## How to use the programs

### The C program

The C program is used as follows:

    ./zeroes <xmin> <xmax> <ymin> <ymax> <xres> <coeff> ... <coeff> - <degree> ... <degree>

It will output a PPM image to the standard output. A typical invocation would be

    ./zeroes -2 2 -1.75 1.75 2000 1 -1 - 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 > picture.ppm

The meaning of the command-line arguments is as follows:

* `<xmin>`, `<xmax>`, `<ymin>`, `<ymax>` define a rectangle in the complex plane that the image will capture
* `xres` is the horizontal size of the image in pixels; the vertical size is computed automatically
* `<coeff> ... <coeff>` a list of floating-point numbers separated by spaces
* `<degree> ... <degree>` a list of integers separated by spaces

The program computes all zeroes of all polynomials of the given degrees with the given
coefficients. It outputs a gray-scale image in the PPM format. The gray levels are
log-scale frequency counts of how many times each pixel was hit by a zero. Thus whiter
regions have a greater density of zeroes.

In order to make a nice picture from the output we need to fiddle with the image colors.
ImageMagick is a wonderful tool for doing precisely that. For instance, given the
`picture.ppm` as computed above, we can run

    convert -normalize -fill orange -tint 100 picture.ppm picture.png
    
to produce a better looking version `picture.png`. For larger images you may need to
replace `-normalize` with a suitable invocatin of `-contrast-stretch`, and for really good
quality you will need some competence with ImageMagick.

If you are planning to generate movies, you can generate a really big picture and then cut
out movie frames from it using ImageMagick. Then you can combine the frames into a movie
with `ffmpeg`.

### The Python program

The Python program `zeroes.py` generates a different kind of picture in which zeroes are
represented as disks of variying radii. The size of the radii decreases exponentially with
the degree. Thus the low-degree zeroes are repreented more prominently.

The program has a usage message, so let me just show a typical usage example:

    ./zeroes.py --out picture.png --size 1000 --radius 200 --degrees 1-14 --coeffs 1,-1 --xmin -2 --xmax 2 --ymin -2 --ymax 2 --colors 0,0,255:0,128,255:128,255,255

And another one:

    ./zeroes.py --out picture.png --size 1000 --radius 100 --degrees 1-10 --coeffs 0,1,2 --xmin -3 --xmax 2 --ymin -2 --ymax 2 --colors 255,255,0:255,128,0:0,255,255

The program `animate.py` uses `zeroes.py` to generate a sequence of images which can then be composed into a movie. The movie shows what happens when we smoothly change the coefficients.
