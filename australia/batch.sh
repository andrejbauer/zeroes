#!/bin/bash

# An example batch script that computes zeros of polynomials,
# stores them in *.dat files, then loads them to create a picture.

opts="--xmin -2 --xmax 2 --ymin -2 --ymax 2 --size 16384"
pic="algebraic-numbers.png"

# These will fail if the output files already exist, so it is safe to
# return the batch, it won't destroy any existing data

./algebraic.py $opts --save roots-1-100.dat --coeff 100 --degrees 1
./algebraic.py $opts --save roots-2-100.dat --coeff 100 --degrees 2
./algebraic.py $opts --save roots-3-40.dat --coeff 40 --degrees 3
./algebraic.py $opts --save roots-4-22.dat --coeff 22 --degrees 4
./algebraic.py $opts --save roots-5-16.dat --coeff 16 --degrees 5
./algebraic.py $opts --save roots-6-13.dat --coeff 13 --degrees 6
./algebraic.py $opts --save roots-7-11.dat --coeff 11 --degrees 7
./algebraic.py $opts --save roots-8-10.dat --coeff 10 --degrees 8
./algebraic.py $opts --save roots-9-9.dat --coeff 9 --degrees 9

# Do not clobber a previously generated picture -- it was a lot of work
if [ -f "$pic" ] ; then mv "$pic" "backup"-`date +%s`-$pic; fi

# Generate the image
./algebraic.py $opts \
 --colors 1,1,0:1,0,0:1,0.5,0:1,0,1:0,0,0.5 \
 --decay 2.9 \
 --load roots-1-100.dat \
 --load roots-2-100.dat \
 --load roots-3-40.dat \
 --load roots-4-22.dat \
 --load roots-5-16.dat \
 --load roots-6-13.dat \
 --load roots-7-11.dat \
 --load roots-8-10.dat \
 --load roots-9-9.dat \
 --draw $pic
