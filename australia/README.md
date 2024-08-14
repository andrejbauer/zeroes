Material for the public talk [Zeros]((https://mathematical-research-institute.sydney.edu.au/event/zeros-andrejbauer/)) at the University of Sydney. There are two programs:

1. The folder [`c-zeros`](./c-zeros/) contains a program that computes zeros and generates large TIFF images, with pixels displaying densities of zeros. It is suitable for generating images of zeros of Littlewood polynomials, see the instructions in the folder.

2. The program [`algebraic.py`](./algebraic.py) computes algebraic numbers for polynomials of given degrees and complexity (where complexity is defined to be the sum of absolute values of the coefficients of a polynomial). It can store computed zeros in a file, load previously computed zeros, and generated images in which zeros are displayed as circles with a gradient. Run `./algebraic --help` for usage information.

Here is a simple example that can get you started with `algebraic.py`. For a more elaborate example, consult [`batch.sh`](./batch.sh):

    ./algebraic.py --coeff 10 --degrees 1,2,3,4,5,6 --colors 1,0,0:0,0,0.5:1,0.75,0 --draw picture.png
