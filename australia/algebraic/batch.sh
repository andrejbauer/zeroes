#!/bin/bash
opts="--xmin -2 --xmax 2 --ymin -2 --ymax 2 --size 16384"
pic="algebraic-numbers.png"

./algebraic.py $opts --save roots-1-100.dat --coeff 100 --degrees 1
./algebraic.py $opts --save roots-2-100.dat --coeff 100 --degrees 2
./algebraic.py $opts --save roots-3-60.dat --coeff 60 --degrees 3
./algebraic.py $opts --save roots-4-45.dat --coeff 35 --degrees 4
./algebraic.py $opts --save roots-5-30.dat --coeff 22 --degrees 5
./algebraic.py $opts --save roots-6-20.dat --coeff 18 --degrees 6
./algebraic.py $opts --save roots-7-15.dat --coeff 15 --degrees 7
./algebraic.py $opts --save roots-8-12.dat --coeff 12 --degrees 8
./algebraic.py $opts --save roots-9-10.dat --coeff 10 --degrees 9

if [ -f "$pic" ] ; then mv "$pic" "backup"-`date +%s`-$pic; fi
./algebraic.py $opts --load roots-1-100.dat --load roots-2-100.dat --load roots-3-60.dat --load roots-4-35.dat --load roots-5-22.dat --load roots-6-18.dat --load roots-7-15.dat --load roots-8-12.dat --load roots-9-10.dat --draw $pic
