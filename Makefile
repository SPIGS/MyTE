CXX=gcc
CFLAGS=-Wall -Wextra -std=c11 -pedantic -ggdb `pkg-config --cflags gl glew glfw3 freetype2`
LDLIBS=-lm `pkg-config --libs gl glew glfw3 freetype2`
TARGET=build/test
SRCS=main.c renderer.c util.c font.c gapbuffer.c editor.c lexer.c
OBJ=$(patsubst %.c, build/%.o, $(SRCS))

all: build $(TARGET)

# Link the object files into the final executable
$(TARGET): $(OBJ)
	$(CXX) $(CFLAGS) -o $@ $(OBJ) $(LDLIBS)

# Compile each source file into an object file in the build directory
build/%.o: %.c
	$(CXX) $(CFLAGS) -c $< -o $@

# Create the build directory if it doesn't exist
build:
	mkdir -p build

clean:
	rm -rf build