PKG_CONFIG_PATH=/usr/local/lib/pkgconfig/
CFILES = ../common/shader.cpp ../common/texture.cpp ../common/controls.cpp main.cpp 
CFLAGS = -O3 -c
INCLUDES = -I../common -I/usr/include -I/usr/include/SOIL -I/opt/local/include -I.  `pkg-config --cflags glfw3` `pkg-config --cflags opencv`
LIBS = -lm -L/usr/local/lib -lGLEW -lSOIL  `pkg-config --static --libs glfw3` `pkg-config --libs opencv`
CC = g++
OBJECTS=$(CFILES:.cpp=.o)
EXECUTABLE=main

all: $(CFILES) $(EXECUTABLE) 

$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(INCLUDES) $(OBJECTS) -o $@ $(LIBS)
.cpp.o:
	$(CC) $(CFLAGS) $(INCLUDES) $< -o $@

clean:
	rm -v -f *~ ../common/*.o *.o *.obj $(EXECUTABLE)
