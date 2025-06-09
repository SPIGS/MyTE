CC=gcc

CFLAGS=-Wall -Wextra -std=c11 -pedantic -ggdb -Iinclude
CFLAGS += $(shell pkg-config --cflags gl glew glfw3 lua freetype2 harfbuzz)

LDLIBS=-lm
LDLIBS += $(shell pkg-config --libs gl glew glfw3 lua libgrapheme freetype2 harfbuzz)

TARGET=myte
SRCS=$(shell find src include -name '*.c')
OBJ=$(patsubst src/%.c, build/%.o, $(SRCS))

UNAME_S := $(shell uname -s)

# MACOS
ifeq ($(UNAME_S), Darwin)
	LDLIBS += -framework OpenGL -L/opt/homebrew/Cellar/lua/5.4.8/lib -llua
	LDLIBS += -L/opt/homebrew/Cellar/freetype/2.13.3/lib -lfreetype
	LDLIBS += -L/opt/homebrew/Cellar/glew/2.2.0_1/lib -lGLEW
	LDLIBS += -L/opt/homebrew/Cellar/glfw/3.4/lib -lglfw
	LDLIBS += -L/opt/homebrew/Cellar/harfbuzz/11.2.1/lib -lharfbuzz
	LDLIBS += -L/opt/homebrew/Cellar/libgrapheme/2.0.2/lib -lgrapheme
	CFLAGS += -I/opt/homebrew/include -I/opt/homebrew/Cellar/lua/5.4.8/include/lua
	CFLAGS += -I/opt/homebrew/Cellar/freetype/2.13.3/include/freetype2
	CFLAGS += -I/opt/homebrew/Cellar/glew/2.2.0_1/include
	CFLAGS += -I/opt/homebrew/Cellar/glfw/3.4/include
	CFLAGS += -I/opt/homebrew/Cellar/harfbuzz/11.2.1/include/harfbuzz -I/opt/homebrew/opt/freetype/include/freetype2 -I/opt/homebrew/opt/libpng/include/libpng16 -I/opt/homebrew/Cellar/glib/2.84.2/include/glib-2.0 -I/opt/homebrew/Cellar/glib/2.84.2/lib/glib-2.0/include -I/opt/homebrew/opt/gettext/include -I/opt/homebrew/Cellar/pcre2/10.45/include -I/opt/homebrew/Cellar/graphite2/1.3.14/include
	CFLAGS += -I/opt/homebrew/include $(shell pkg-config --cflags libgrapheme)

endif

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
