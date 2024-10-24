CXX=gcc
CFLAGS=-Wall -Wextra -std=c11 -pedantic -ggdb `pkg-config --cflags gl glew glfw3 freetype2`
LDLIBS=-lm `pkg-config --libs gl glew glfw3 freetype2`
TARGET=myte
SRCS=$(addprefix src/, main.c application.c renderer.c util.c font.c gapbuffer.c editor.c lexer.c toml.c config.c  browser.c keys.c cursor.c dialog.c)
OBJ=$(patsubst src/%.c, build/%.o, $(SRCS))

all: clean build $(TARGET)

# Link the object files into the final executable
$(TARGET): $(OBJ)
	$(CXX) $(CFLAGS) -o $@ $(OBJ) $(LDLIBS)

# Compile each source file into an object file in the build directory
build/%.o: src/%.c
	$(CXX) $(CFLAGS) -c $< -o $@

# Create the build directory if it doesn't exist
build:
	mkdir -p build

# Clean up
clean:
	rm -rf build