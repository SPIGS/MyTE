CXX=gcc
CFLAGS=-Wall -Wextra -std=c11 -pedantic -ggdb `pkg-config --cflags gl glew glfw3 freetype2`
LDLIBS=-lm `pkg-config --libs gl glew glfw3 freetype2`
TARGET=test
SRCS=main.c renderer.c util.c font.c
OBJ=$(subst .c,.o,$(SRCS))

all: $(OBJ)
	$(CXX) $(CFLAGS) -o $(TARGET) $(OBJ) $(LDLIBS)

main.o: main.c
	$(CXX) $(CFLAGS) -c main.c

renderer.o: renderer.c renderer.h util.h font.h
	$(CXX) $(CFLAGS) -c renderer.c

util.o: util.c util.h
	$(CXX) $(CFLAGS) -c util.c

font.o: font.c font.h
	$(CXX) $(CFLAGS) -c font.c

clean:
	rm -f *.o