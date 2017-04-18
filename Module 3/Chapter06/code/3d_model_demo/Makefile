PKG_CONFIG_PATH=/usr/local/lib/pkgconfig/
CFILES = ../common/shader.cpp ../common/controls.cpp ../common/ObjLoader.cpp main.cpp 
CFLAGS = -c
OPT = -O3
INCLUDES = -I../common -I/usr/include -I/opt/local/include -I/usr/include/SOIL -I.  `pkg-config --cflags glfw3` `pkg-config --cflags assimp`
LIBS = -lm -L/usr/local/lib -lGLEW `pkg-config --static --libs glfw3` `pkg-config --static --libs assimp`
CC = g++
OBJECTS=$(CFILES:.cpp=.o)
EXECUTABLE=main

all: $(CFILES) $(EXECUTABLE) 

$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(OPT) $(INCLUDES) $(OBJECTS) -o $@ $(LIBS)
.cpp.o:
	$(CC) $(OPT) $(CFLAGS) $(INCLUDES) $< -o $@

clean:
	rm -v -f *~ ../common/*.o *.o $(EXECUTABLE)
