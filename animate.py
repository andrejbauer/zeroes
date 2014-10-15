#!/usr/bin/python

from zeroes import Zeroes

def line(a,b,n):
    """Retun a list of n points on the segment between points a and b."""
    (x0,y0) = a
    (x1,y1) = b
    lst = []
    for i in range(n):
        t = i/(n-1.0)
        x = (1.0 - t) * x0 + t * x1
        y = (1.0 - t) * y0 + t * y1
        lst.append((x,y))
    return lst

for (i,c) in enumerate(line((1.0, 1.0), (-1.0, 1.0), 300)):
    nicle = Zeroes(xmin=-2.25, xmax=1.75, ymin=-2.0, ymax=2.0, coeffs=c,
                   xres=800, degrees=tuple(range(12)), colors=((0,0,255),(255,255,255)))
    nicle.draw_roots()
    filename = "movie{:03d}.png".format(i)
    print("Saving {0}".format(filename))
    with open(filename, "wb") as f: nicle.save_image(f)

