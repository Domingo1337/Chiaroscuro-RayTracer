# Przykladowy Makefile dla tutorialu06 dzialajacy na pracowni
# (c) anl 2015-2017 
#
CXX=g++
CFLAGS=-I/home/anl/PGK/include/ -I/home/anl/PGK/include/GLFW/ -I/usr/local/include/GLFW/ -I/home/synthwave/Projects/OpenGLTutorials/external/ -I./include/ -I./ -L../build/external/glfw-3.1.2/src -std=c++11 -L../build/external/ -L/home/anl/PGK/lib
LIBS=-lglfw3 -lGL  -ldl -lX11 -lXxf86vm -lXrandr -lXi -lXinerama -lXcursor -lpthread -lassimp
#COMMON=/home/anl/PGK/lib/shader.o /home/anl/PGK/lib/controls.o  /home/anl/PGK/lib/texture.o

default: main

.cpp.o:
	${CXX} -c ${CFLAGS} $<

main: main.o glad.o
	${CXX} -L../build/external/ -L/home/anl/PGK/lib/ main.o glad.o -o main ${COMMON} ${LIBS}

main.o: main.cpp
	${CXX} ${CFLAGS} -L/home/anl/PGK/lib/ -c main.cpp -o main.o   ${COMMON} ${LIBS}

glad.o: src/glad.c
	${CXX} ${CFLAGS} -L/home/anl/PGK/lib/ -c src/glad.c -o glad.o  ${COMMON} ${LIBS}

clean:
	rm main.o glad.o main


