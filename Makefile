all: clean main.cpp
	g++ main.cpp -o modelviewer -I/usr/include -L/usr/lib -lglut -lGL -lGLU -lX11

clean:
	rm -f modelviewer
