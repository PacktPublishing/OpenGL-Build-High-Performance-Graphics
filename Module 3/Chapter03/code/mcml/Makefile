CFILES = main.cpp
CFLAGS = -O3 -Wall `pkg-config --cflags glfw3`
CC = gcc

all: main
main:	$(CFILES)
	$(CC) $(CFLAGS) -o main $(CFILES) -lm `pkg-config --static --libs glfw3`

clean:
	rm -f *~ *.o *.obj main
