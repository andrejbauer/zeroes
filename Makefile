# OSX
zeroes: zeroes.c
	gcc -O3 -o zeroes -lgsl zeroes.c

# Linux
#zeroes: zeroes.c
#	gcc -std=c99 -O3 -o zeroes -lgsl -L/usr/lib/openblas-base/ -lopenblas zeroes.c
