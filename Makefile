CC=gcc
CFLAGS=-Wall -Wextra -std=c11 -pedantic -ggdb `pkg-config --cflags gl glew glfw3 lua` -Iinclude
LDLIBS=-lm `pkg-config --libs gl glew glfw3 lua libgrapheme`
TARGET=myte
SRCS=$(shell find src include -name '*.c')
OBJ=$(patsubst src/%.c, build/%.o, $(SRCS))

all: clean build $(TARGET)

# Link the object files into the final executable
$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJ) $(LDLIBS)

# Compile each source file into an object file in the build directory
build/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Create the build directory if it doesn't exist
build:
	mkdir -p build

# Clean up
clean:
	rm -rf build/*.o $(TARGET)
