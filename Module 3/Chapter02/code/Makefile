CFLAGS = -O3 -Wall `pkg-config --cflags glfw3`

all: ecg_demo gaussian_demo sinusoid_demo point line triangle

ecg_demo:	Tutorial2/ecg/main_ecg_demo.cpp
	$(CC) $(CFLAGS) -o ecg_demo Tutorial2/ecg/main_ecg_demo.cpp -lm `pkg-config --static --libs glfw3`

gaussian_demo:	Tutorial2/gaussian/main_gaussian_demo.cpp
	$(CC) $(CFLAGS) -o gaussian_demo Tutorial2/gaussian/main_gaussian_demo.cpp -lm `pkg-config --static --libs glfw3`

point:		Tutorial2/point/main_point.cpp
	$(CC) $(CFLAGS) -o point Tutorial2/point/main_point.cpp  -lm `pkg-config --static --libs glfw3`

sinusoid_demo:	Tutorial2/sinusoid/main_sinusoid.cpp
	$(CC) $(CFLAGS) -o sinusoid_demo Tutorial2/sinusoid/main_sinusoid.cpp -lm `pkg-config --static --libs glfw3`

line: 		Tutorial2/line/main_line.cpp
	$(CC) $(CFLAGS) -o line	Tutorial2/line/main_line.cpp -lm `pkg-config --static --libs glfw3`

triangle:	Tutorial2/triangle/main_triangle.cpp
	$(CC) $(CFLAGS) -o triangle Tutorial2/triangle/main_triangle.cpp -lm `pkg-config --static --libs glfw3`


clean:
	rm -f *~ *.o *.obj ecg_demo gaussian_demo sinusoid_demo point line triangle
