#!/usr/bin/python
from __future__ import division
import math
import sys
import subprocess

def interpolate(a, x, y):
    """Interpolate between x and y. When a is 0 the value is x, when a is 1 the value is y."""
    return (1 - a) * x + a * y

def geometry(w, h, i, j):
    """Convert a rectangle to ImageMagick geometry"""
    return "{0}x{1}+{2}+{3}".format(w, h, i - w//2, j - (h-1)//2)

class Flyover():
    """Store information about the large image from which we make a movie."""
    def __init__(self, xmin, xmax, ymin, ymax, xres_orig, orig, xres_movie, yres_movie, rate=30):
        # Which part of the plane the big image shows
        self.xmin = xmin
        self.xmax = xmax
        self.ymin = ymin
        self.ymax = ymax
        # Animation rate (FPS)
        self.rate = rate
        # Original image file
        self.orig = orig
        # Resolution of the big image
        self.xres = xres_orig
        self.yres = int ((self.xres * (ymax-ymin)) / (xmax-xmin))
        # Movie resolution
        self.xmovie = xres_movie
        self.ymovie = yres_movie
        self.path = {}

    def pixel(self, x, y):
        """Convert plane coordinates to pixel coordinates"""
        i = int(self.xres * (x - self.xmin) / (self.xmax - self.xmin))
        j = self.yres - 1 - int(self.yres * (y - self.ymin) / (self.ymax - self.ymin))
        return (i,j)

    def get_size(self, scale):
        w1 = max (self.xres, self.xmovie * self.yres / self.ymovie)
        h1 = max (self.yres, self.ymovie * self.xres / self.xmovie)
        w = min (self.xres, int(interpolate (scale, self.xmovie, w1)))
        h = min (self.yres, int(interpolate (scale, self.ymovie, h1)))
        return (w,h)

    def linear(self, x0, y0, x1, y1, scale0, scale1, time, out):
        """Compute a list of geometries for moving from (x0,y0) to (x1,y1) in time seconds.
           The scale0 and scale1 parameters tell how much to zoom in at (x0,y0) and (x1,y1),
           respectively. A scale value of 1 means 'original image size' and 0 means 'maximum zoom'.
           The out parameter is the name of the folder where the images will be stored. The out
           parameter is also used as a reference by the magick method below."""
        frames = int(time * self.rate)
        lst = []
        for i in range(0, frames+1):
            a = float(i)/frames
            x = interpolate (a, x0, x1)
            y = interpolate (a, y0, y1)
            scale = interpolate (a, scale0, scale1)
            (w, h) = self.get_size(scale)
            (i, j) = self.pixel(x, y)
            lst.append(geometry(w, h, i, j))
        # Repeat the last frame, otherwise Keynote does a little "jitter" thingy
        # lst.append(geometry(w, h, i, j))
        # Store the computed path for alter use
        lst.append(geometry(w, h, i, j))
        self.path[out] = lst

    def arc(self, phi0, r0, phi1, r1, scale0, scale1, time, out):
        """Compute a list of geometries for moving from (phi0,r0) to (phi1,r1) in polar
           coordinates in time seconds. We interpolate the angle and the radius linearly.
           The scale0 and scale1 parameters tell how much to zoom in at (x0,y0) and (x1,y1),
           respectively. A scale value of 1 means 'original image size' and 0 means 'maximum zoom'.
           The out parameter is the name of the folder where the images will be stored. The out
           parameter is also used as a reference by the magick method below."""
        frames = int(time * self.rate)
        lst = []
        for i in range(0, frames+1):
            a = float(i)/frames
            r = interpolate (a, r0, r1)
            phi = interpolate (a, phi0, phi1)
            scale = interpolate (a, scale0, scale1)
            x = r * math.cos(phi)
            y = r * math.sin(phi)
            (w, h) = self.get_size(scale)
            (i, j) = self.pixel(x, y)
            lst.append(geometry(w, h, i, j))
        # Repeat the last frame, otherwise Keynote does a little "jitter" thingy
        # lst.append(geometry(w, h, i, j))
        # Store the computed path for alter use
        self.path[out] = lst

    def magick(self, out):
        """Generate ImageMagick cropping commands, feed them into ImageMagic and run ffmpeg
           to generate the movie. The out parameter is one of the folders."""
        lst = self.path[out]
        convert = subprocess.Popen(['convert','-verbose', '@-'],
                                   stdin=subprocess.PIPE, stdout=0, stderr=0)
        convert.stdin.write("{0} -write mpr:orig\n".format(self.orig))
        for (i, geom) in enumerate(lst):
            convert.stdin.write("+delete mpr:orig -gravity None -crop {0} -resize {1}x{2} -gravity Center -background black -extent {1}x{2} -write {3}/pic{4:04d}.png\n".
                    format(geom, self.xmovie, self.ymovie, out, i))
        convert.stdin.write("null:\n")
        convert.stdin.close()
        convert.wait()
        ffmpeg = subprocess.Popen(['ffmpeg',
                                  '-r', str(self.rate),
                                  '-i', '{0}/pic0%03d.png'.format(out),
                                  '-r', str(self.rate),
                                  '-b:v', '10000k',
                                  '{0}.mpg'.format(out)],
                                  stdin=1, stdout=0, stderr=0)
        ffmpeg.wait()
        print ("FINISHED")

#### Main program

# We define the flyover object. This one is for the official 20000x17500 image.
# If you are going to make your own movie, I recommend that you first make a
# scaled down version of the image and experiment with it first.
flyover = Flyover(
    orig = "zeroes26.png",
    xmin = -2.0,
    xmax = 2.0,
    ymin = -1.75,
    ymax = 1.75,
    xres_orig = 20000,
    xres_movie = 1920, # Full HD
    yres_movie = 1080)

# Fly into (0,1)
flyover.linear(x0=0,y0=0,
             x1=0,y1=1,
             scale0=1,
             scale1=0,
             time=10,
             out='zoom')

# From (0,1) to (1/sqrt(2), 1/sqrt(2))
flyover.arc(
    r0=1.0,
    phi0=math.pi/2,
    r1=1.0,
    phi1=math.pi/3,
    scale0=0,
    scale1=0,
    time=10,
    out='arc')

# From (1/sqrt(2), 1/sqrt(2)) to top
flyover.linear(
    x0 = math.cos(math.pi/3),
    y0 = math.sin(math.pi/3),
    x1 = 0,
    y1 = 1.4,
    scale0=0,
    scale1=0,
    time=5,
    out='tofringe')

# Arc on the fringe to pi/3
flyover.arc(
    r0=1.4,
    phi0=math.pi/2,
    r1=1.5,
    phi1=7*math.pi/24,
    scale0=0,
    scale1=0.2,
    time=10,
    out='fringe')

# Rest of the arc into (0,0)
flyover.arc(
    r0=1.5,
    phi0=7*math.pi/24,
    r1=1,
    phi1=0,
    scale0=0.2,
    scale1=0.15,
    time=10,
    out='tozero')

# Zoom into zero
flyover.linear(
    x0=1,
    y0=0,
    x1=1,
    y1=0,
    scale0=0.15,
    scale1=0.0,
    time=5,
    out='atzero')

# Zoom back out
flyover.linear(
    x0=1,
    y0=0,
    x1=0,
    y1=0,
    scale0=0.0,
    scale1=1.0,
    time=1,
    out='unzoom')

# To use the program, we give it one of the above out parameters
# on the command line, but we first make sure that the folder
# where the images are stored exists:
#
#    mkdir -p fringe
#    rm -f fringe/pic*.png
#    rm fringe.mpg
#    python movie.py fringe
#
flyover.magick(sys.argv[1])
