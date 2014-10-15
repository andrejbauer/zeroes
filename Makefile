# Configure the libraries to be used
# This seems to work on OSX
LIBS=-lgsl
# And this works on Linux
# LIBS=-lgsl -L/usr/lib/openblas-base/ -lopenblas

# OSX
zeroes: zeroes.c
	gcc -std=c99 -O3 -o zeroes $(LIBS) zeroes.c

clean:
	/bin/rm -f zeroes *.pyc
