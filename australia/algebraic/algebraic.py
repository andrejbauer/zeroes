#!/usr/local/bin/python3

# Compute algebraic numbers in the complex plane and draw a nice picture

import numpy
import sys
import argparse
import math
import cairo
import pickle

def degree_list(s):
    """Convert degrees given on command line to a list.
       For example, the string '1,2-5,7' is converted to [1,2,3,4,5,7]."""
    l = []
    for r in s.split(','):
        t = r.split('-')
        if len(t) == 1:
            l.append(int(t[0]))
        else:
            a = int(t[0])
            b = int(t[1])
            l.extend(range(a,b+1))
    return l

def weight(poly):
    return sum(map(abs,poly))

def importance(poly):
    return (weight(poly), len(poly))

def int_list(bound):
    """List of floats from -bound to bound, to be used as coefficients."""
    return list(range(-bound, bound+1))

def color_list(s):
    """Convert a colon-separated list of RGB triples to a list of triples.
       Example: the string '255,127,0:127,127,127:0,0,255' is converted
       to [(255,127,0), (127,127,127), (0,0,255)]."""
    return (tuple(tuple(map(float,rgb.split(','))) for rgb in s.split(':')))


## Color function helper

def compute_colors(n, cols):
    """Interpolate a list of colors cols to a list of n colors."""
    if n == 1:
        return (cols[0],)
    else:
        m = len(cols)
        lst = []
        for i in range(n):
            j = math.floor (i * (m - 1.0) / (n - 1.0))
            k = math.ceil (i * (m - 1.0) / (n - 1.0))
            t = (i * (m - 1.0) / (n - 1.0)) - j
            (r0, g0, b0) = cols[int(j)]
            (r1, g1, b1) = cols[int(k)]
            r = min(1.0, max(0.0, (1.0 - t) * r0 + t * r1))
            g = min(1.0, max(0.0, (1.0 - t) * g0 + t * g1))
            b = min(1.0, max(0.0, (1.0 - t) * b0 + t * b1))
            lst.append((r,g,b))
        return tuple(lst)

## Main class to represent an image
class AlgebraicNumbers():
    """Representation of all the data needed to calculate the scene."""

    def __init__(self,
                 xmin = -1.5, xmax = 1.5, ymin = -1.5, ymax = 1.5, # rectangle in the complex plane
                 xres = 512, yres=None, # image resolution (yres is automatically calculated if ommitted)
                 radius = 1.0, # radius of largest circle
                 decay = 0.5, # exponent by which the radius decreeses
                 save = False, # should we save the roots?
                 colors = ((1,0,0), (0,1,0), (0,0,1)) # list of colors to use to draw zeroes
    ):
        # Store parameters
        self.xmin = xmin
        self.xmax = xmax
        self.ymin = ymin
        self.ymax = ymax
        self.radius = radius
        self.decay = decay
        self.colors = colors
        self.save = save
        # Precalculate stuff
        self.dx = xmax - xmin
        self.dy = ymax - ymin
        self.xres = xres
        self.yres = yres or int(xres * self.dy / self.dx)
        self.degree_min = 1000000000000
        self.degree_max = -1
        self.roots = [] # all roots
        self.stars = {} # the roots that we are going to draw

    def star(self, ctx, x, y, size, c):
        r, g, b = c
        shine = cairo.RadialGradient(x,y,0, x,y,size)
        shine.add_color_stop_rgba(0.0,  1,1,1, 1.0)
        shine.add_color_stop_rgba(0.05, r,g,b, 1.0)
        shine.add_color_stop_rgba(0.25, r,g,b, 1.0)
        shine.add_color_stop_rgba(1.0,  r,g,b, 0.0)
        ctx.arc(x,y,size,0,2*math.pi)
        ctx.set_source(shine)
        ctx.fill()


    def register(self, real, imag, poly):
        """Register a root."""
        if self.save: self.roots.append((real, imag, tuple(poly)))
        degree = len(poly) - 1
        self.degree_min = min(self.degree_min, degree)
        self.degree_max = max(self.degree_max, degree)
        i = round ((real - self.xmin) / self.dx * self.xres)
        j = round ((imag - self.xmax) / self.dy * self.yres)
        if ((i,j) not in self.stars) or importance(poly) < importance(self.stars[(i,j)][2]):
            self.stars[(i,j)] = (real, imag, tuple(poly))

    def compute(self, degree, max_coeff):
        """Compute the algebraic numbers to be drawn."""
        poly = list(range(0,degree+1))

        def loop(k, coeff):
            if k <= degree:
                cmax = coeff - degree + k
                cmin = (-cmax if k != 0 else 1)
                for c in range(cmin, cmax+1):
                    if k == 0: print ("Degree {0}: {1}/{2}".format(degree, c, coeff), end='\r')
                    if k == degree and degree > 1 and c == 0: continue # constant term must be non-zero for non-linear polynomials
                    poly[k] = c
                    loop(k+1, coeff - abs(c))
            else:
                for root in numpy.roots(poly):
                    self.register(root.real, root.imag, poly)

        loop(0, max_coeff)
        print()

    def draw(self):
        """Draw all roots of polynomials whose sum of absolute values does not exceed
           self.coeff and whose degree does not exceed self.degree.."""
        i = 0
        m = len(self.stars)
        colors = compute_colors(self.degree_max - self.degree_min + 1, self.colors)
        # Create image and canvas
        self.image = cairo.ImageSurface(cairo.FORMAT_ARGB32, self.xres, self.yres)
        ctx = cairo.Context(self.image)
        ctx.scale(self.xres / self.dx, self.yres / self.dy)
        ctx.translate(-self.xmin, -self.ymin)
        # Paint a black background
        ctx.set_source_rgb(0,0,0)
        ctx.rectangle(self.xmin, self.ymin, self.dx, self.dy)
        ctx.fill()
        for (x, y, poly) in sorted(self.stars.values(), reverse=True, key=(lambda r: len(r[2]))):
            d = len(poly) - 1
            col = colors[d-self.degree_min] # color
            # r = max(0.0001, self.radius * (self.decay ** weight(poly))) # radius
            r = max(0.0001, self.radius / weight(poly) ** self.decay) # radius
            self.star(ctx, x, y, r, col)
            i += 1
            if i % 1000 == 0:
                print ("Drawing roots: {0}%   ".format(round(100 * i / m)), end='\r')

    def save_numbers(self, fh):
        """Save the computed roots to a file."""
        if not self.save:
            print ("Nothing to save.")
        else:
            print ("Saving {0} roots to {1}".format(len(self.roots), fh.name))
            pickle.dump(self.roots, fh)

    def load_numbers(self, fh):
        """Load precomputed roots from a file."""
        roots = pickle.load(fh)
        print ("Loading {0} roots from {1}".format(len(roots), fh.name))
        for (real, imag, poly) in roots:
            self.register(real, imag, poly)

    def save_image(self, outfile):
        """Save image to the given output file in PNG format."""
        self.image.write_to_png(outfile)

# Main program
if __name__ == '__main__':
    ## Process command line
    parser = argparse.ArgumentParser(description = "Generate images of complex zeroes")
    parser.add_argument('--load', dest='load', action='append', type=argparse.FileType('rb'), help='file to load precomputed zeroes')
    parser.add_argument('--save', dest='save', type=argparse.FileType('xb'), help='file to save computed zeroes')
    parser.add_argument('--draw', dest='draw', type=argparse.FileType('xb'), help='output file (PNG)')
    parser.add_argument('--size', dest='size', default=1024, type=int, help='horizontal image size in pixels')
    parser.add_argument('--radius', dest='radius', default=0.5, type=float, help='maximum root radius')
    parser.add_argument('--decay', dest='decay', default=2.5, type=float, help='radius decay factor')
    parser.add_argument('--degrees', dest='degrees', default=[], type=degree_list, help='degrees to compute')
    parser.add_argument('--coeff', dest='coeff', default=10, type=int, help='bound on sum of coefficients')
    parser.add_argument('--xmin', dest='xmin', default=-2.0, type=float, help='minimum real component')
    parser.add_argument('--xmax', dest='xmax', default= 2.0, type=float, help='maximum real component')
    parser.add_argument('--ymin', dest='ymin', default=-2.0, type=float, help='minimum imaginary component')
    parser.add_argument('--ymax', dest='ymax', default= 2.0, type=float, help='maximum imaginary component')
    parser.add_argument('--colors', dest='colors', default=((1,1,0),(1,0.5,0),(1,0,0.5),(0,0,1)), type=color_list, help='list of colors')
    args = parser.parse_args()
    if not (args.save or args.draw):
        print ("It makes no sense to use neither --save nor --draw")
        exit(1)
    nums = AlgebraicNumbers(xmin=args.xmin, xmax=args.xmax, ymin=args.ymin, ymax=args.ymax,
                            xres=args.size, radius=args.radius, decay=args.decay, save=(bool(args.save)),
                            colors=args.colors)
    if args.load:
        for fh in args.load:
            nums.load_numbers(fh)
    else:
        print ("Computing numbers ...")
        for degree in args.degrees:
            nums.compute(degree, args.coeff)
    if args.save:
        nums.save_numbers(args.save)
    if args.draw:
        print ("Drawing {0} numbers to {1}...".format(len(nums.stars), args.draw.name))
        nums.draw()
        nums.save_image(args.draw)
