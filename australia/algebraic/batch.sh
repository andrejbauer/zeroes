#!/bin/bash
size=1100
opts="--xmin -2.2 --xmax 2.2 --ymin -1.5 --ymax 1.5 --size 11000"
pic="algebraic-numbers.png"

./algebraic.py $opts --save roots-1-100.dat --coeff 100 --degrees 1
./algebraic.py $opts --save roots-2-100.dat --coeff 100 --degrees 2
./algebraic.py $opts --save roots-3-40.dat --coeff 40 --degrees 3
./algebraic.py $opts --save roots-4-25.dat --coeff 25 --degrees 4
./algebraic.py $opts --save roots-5-20.dat --coeff 20 --degrees 5

if [ -f "$pic" ] ; then mv "$pic" "backup"-`date +%s`-$pic; fi
./algebraic.py $opts --load roots-1,2-100.dat --load roots-3-40.dat --load roots-4-25.dat --load roots-5-20.dat --draw algebraic-numbers.png
