CXX=gcc
CFLAGS=-Wall -Wextra -std=c11 -pedantic -ggdb `pkg-config --cflags gl glew glfw3 freetype2`
LDLIBS=`pkg-config --libs gl glew glfw3 freetype2`
TARGET=test
SRCS=main.c gapbuffer.c
OBJ=$(subst .c,.o,$(SRCS))

all: $(OBJ)
	$(CXX) $(CFLAGS) -o $(TARGET) $(OBJ) $(LDLIBS)

main.o: main.c gapbuffer.h
	$(CXX) $(CFLAGS) -c main.c

gapbuffer.o: gapbuffer.c gapbuffer.h
	$(CXX) $(CFLAGS) -c gapbuffer.c

clean:
	rm -f *.o