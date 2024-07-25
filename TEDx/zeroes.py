#!/usr/bin/python
# -*- coding: utf-8 -*-

import numpy
import sys
import argparse
import math
from PIL import Image, ImageDraw

def degreeList(s):
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
            l.extend(range(a,b+1, (1 if a <= b else -1)))
    return sorted(l)

def float_list(s):
    """Convert a string of comma separated floats to a list of floats."""
    return sorted(map (float, s.split(',')))

def color_list(s):
    """Convert a list of RGB components to a list of triples.
       Example: the string '255,127,0:127,127,127:0,0,255' is converted
       to [(255,127,0), (127,127,127), (0,0,255)]."""
    return (map(int,rgb.split(',')) for rgb in s.split(':'))


## Color function helper

def compute_colors(n, cols):
    """Interpolate a list of colors cols to a list of n colors."""
    m = len(cols)
    lst = []
    for i in range(n):
        j = math.floor (i * (m - 1.0) / (n - 1.0))
        k = math.ceil (i * (m - 1.0) / (n - 1.0))
        t = (i * (m - 1.0) / (n - 1.0)) - j
        (r0, g0, b0) = cols[int(j)]
        (r1, g1, b1) = cols[int(k)]
        r = min(255, max(0, int (0.5 + (1.0 - t) * r0 + t * r1)))
        g = min(255, max(0, int (0.5 + (1.0 - t) * g0 + t * g1)))
        b = min(255, max(0, int (0.5 + (1.0 - t) * b0 + t * b1)))
        lst.append((r,g,b))
    return lst

## Main class to represent an image
class Zeroes():
    """Representation of all the data needed to calculate an image."""

    def __init__(self,
                 coeffs, # list of coefficient values
                 degrees, # list of degrees
                 xmin = -1.5, xmax = 1.5, ymin = -1.5, ymax = 1.5, # rectangle in the complex plane
                 xres = 512, yres=None, # image resolution (yres is automatically calculated if ommitted)
                 radius = 200.0, # radius of circle representing degree 0
                 colors = ((255,0,0), (0,255,0), (0,0,255)) # list of colors to use to draw zeroes
    ):
        # Store parameters
        self.xmin = xmin
        self.xmax = xmax
        self.ymin = ymin
        self.ymax = ymax
        self.degrees = degrees
        self.coeffs = coeffs
        self.radius = radius
        # Precalculate stuff
        self.dx = xmax - xmin
        self.dy = ymax - ymin
        self.xres = xres
        self.yres = yres or int(xres * self.dy / self.dx)
        self.dmin = min(degrees)
        self.dmax = max(degrees)
        self.colors = compute_colors(1 + self.dmax - self.dmin, tuple(colors))
        # Create image and canvas
        self.image = Image.new("RGB", (self.xres, self.yres))
        self.draw = ImageDraw.Draw(self.image)

    def draw_roots1(self, degree):
        """Draw the roots of polynomials of the given degree."""
        poly = list(range(0,degree))
        r = max (0.5, self.radius * (0.55 ** degree)) # radius
        col = self.colors[degree - self.dmin] # color
        def loop(k):
            if k < degree:
                for c in self.coeffs:
                    poly[k] = c
                    loop(k+1)
            else:
                for root in numpy.roots(poly):
                    px = self.xres * (root.real - self.xmin) / self.dx
                    py = self.yres * (root.imag - self.ymin) / self.dy
                    self.draw.ellipse(
                        [int(px-r+0.5), int(py-r+0.5), int(px+r+0.5), int(py+r+0.5)],
                        fill=col)

        loop(0)

    def draw_roots(self):
        """Draw all roots of polynomials of all the given degrees."""
        for d in self.degrees:
            print ("Computing degreee {0}".format(d))
            self.draw_roots1(d)

    def save_image(self, outfile):
        """Save image to the given output file in PNG format."""
        self.image.save(outfile, 'PNG', dpi = (300, 300))

# Main program
if __name__ == '__main__':
    ## Process command line
    parser = argparse.ArgumentParser(description = "Generate images of complex zeroes")
    parser.add_argument('--out', dest='outfile', required=True, type=argparse.FileType('w'), help='output file (PNG)')
    parser.add_argument('--size', dest='size', default=512, type=int, help='horizontal image size in pixels')
    parser.add_argument('--radius', dest='radius', default=100.0, type=float, help='maximum root radius')
    parser.add_argument('--degrees', dest='degrees', required=True, type=degreeList, help='polynomial degrees')
    parser.add_argument('--coeffs', dest='coeffs', required=True, type=float_list, help='polynomial coefficients')
    parser.add_argument('--xmin', dest='xmin', default=-3.0, type=float, help='minimum real component')
    parser.add_argument('--xmax', dest='xmax', default= 3.0, type=float, help='maximum real component')
    parser.add_argument('--ymin', dest='ymin', default=-3.0, type=float, help='minimum imaginary component')
    parser.add_argument('--ymax', dest='ymax', default= 3.0, type=float, help='maximum imaginary component')
    parser.add_argument('--colors', dest='colors', default=((255,0,0),(0,255,0),(0,0,255)), type=color_list, help='list of colors')
    args = parser.parse_args()
    nicle = Zeroes(xmin=args.xmin, xmax=args.xmax, ymin=args.ymin, ymax=args.ymax,
                  coeffs=args.coeffs,
                  xres=args.size, radius=args.radius, degrees=args.degrees, colors=args.colors)
    nicle.draw_roots()
    nicle.save_image(args.outfile)
