# Przykladowy Makefile dla tutorialu06 dzialajacy na pracowni
# (c) anl 2015-2017 
#
CXX= g++ -fopenmp
CFLAGS= -I/usr/local/include/GLFW/ -I./include/ -I./  -std=c++14  -Wall -Wextra -O3
LIBS=-lglfw3 -lGL  -ldl -lX11 -lXxf86vm -lXrandr -lXi -lXinerama -lXcursor -lpthread -lassimp -lfreeimage

default: main

.cpp.o:
	${CXX} -c ${CFLAGS} $<

main: main.o glad.o scene.o mesh.o model.o openglPreview.o camera.o rayTracer.o shader.o kdtree.o material.o
	${CXX} -Wall -Wextra main.o glad.o mesh.o scene.o model.o openglPreview.o camera.o rayTracer.o shader.o kdtree.o material.o -o main ${COMMON} ${LIBS}

main.o: main.cpp
	${CXX} ${CFLAGS} -c main.cpp -o main.o   ${COMMON} ${LIBS}

glad.o: src/glad.c
	${CXX} ${CFLAGS} -c src/glad.c -o glad.o  ${COMMON} ${LIBS}


scene.o: src/scene.cpp
	${CXX} ${CFLAGS} -c src/scene.cpp -o scene.o  ${COMMON} ${LIBS}

mesh.o: src/mesh.cpp
	${CXX} ${CFLAGS} -c src/mesh.cpp -o mesh.o  ${COMMON} ${LIBS}

model.o: src/model.cpp
	${CXX} ${CFLAGS} -Wno-implicit-fallthrough -c src/model.cpp -o model.o  ${COMMON} ${LIBS}

openglPreview.o: src/openglPreview.cpp
	${CXX} ${CFLAGS} -Wno-unused-parameter -c src/openglPreview.cpp -o openglPreview.o  ${COMMON} ${LIBS}

camera.o: src/camera.cpp
	${CXX} ${CFLAGS} -c src/camera.cpp -o camera.o  ${COMMON} ${LIBS}

rayTracer.o: src/rayTracer.cpp
	${CXX} ${CFLAGS} -c src/rayTracer.cpp -o rayTracer.o  ${COMMON} ${LIBS}

shader.o: src/shader.cpp
	${CXX} ${CFLAGS} -c src/shader.cpp -o shader.o  ${COMMON} ${LIBS}

kdtree.o: src/kdtree.cpp
	${CXX} ${CFLAGS} -c src/kdtree.cpp -o kdtree.o  ${COMMON} ${LIBS}

material.o: src/material.cpp
	${CXX} ${CFLAGS} -Wno-unused-parameter -c src/material.cpp -o material.o  ${COMMON} ${LIBS}

clean:
	rm -f main *.o


