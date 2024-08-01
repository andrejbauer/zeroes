# Compiler and linker
CC = clang

# Compiler flags
CFLAGS = -Wall -Wextra -I/opt/homebrew/opt/gsl/include -I/opt/homebrew/opt/libtiff/include -I.

# Linker flags
LDFLAGS = -L/opt/homebrew/opt/gsl/lib -L/opt/homebrew/opt/libtiff/lib -lgsl -lgslcblas -ltiff -lm -pthread

# Targets and dependencies
TARGET = zeros
OBJS = zeros.o compute.o image.o

# Default target
all: $(TARGET)

# Build the target
$(TARGET): $(OBJS)
	$(CC) -o $@ $(OBJS) $(LDFLAGS)

# Compile source files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up build artifacts
clean:
	rm -f $(TARGET) $(OBJS) output.tiff

# Phony targets
.PHONY: all clean
