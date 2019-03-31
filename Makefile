# Przykladowy Makefile dla tutorialu06 dzialajacy na pracowni
# (c) anl 2015-2017 
#
CXX=g++
CFLAGS= -I/usr/local/include/GLFW/ -I./include/ -I./ -L../build/external/glfw-3.1.2/src -std=c++11 -L../build/external/ -Wall -Wextra
LIBS=-lglfw3 -lGL  -ldl -lX11 -lXxf86vm -lXrandr -lXi -lXinerama -lXcursor -lpthread -lassimp -lfreeimage
#COMMON=/home/anl/PGK/lib/shader.o /home/anl/PGK/lib/controls.o  /home/anl/PGK/lib/texture.o

default: main

.cpp.o:
	${CXX} -c ${CFLAGS} $<

main: main.o glad.o scene.o mesh.o model.o openglPreview.o camera.o rayCaster.o shader.o
	${CXX} -Wall -Wextra -L../build/external/ -L/home/anl/PGK/lib/ main.o glad.o mesh.o scene.o model.o openglPreview.o camera.o rayCaster.o shader.o -o main ${COMMON} ${LIBS}

main.o: main.cpp
	${CXX} ${CFLAGS} -L/home/anl/PGK/lib/ -c main.cpp -o main.o   ${COMMON} ${LIBS}

glad.o: src/glad.c
	${CXX} ${CFLAGS} -L/home/anl/PGK/lib/ -c src/glad.c -o glad.o  ${COMMON} ${LIBS}


scene.o: src/scene.cpp
	${CXX} ${CFLAGS} -L/home/anl/PGK/lib/ -c src/scene.cpp -o scene.o  ${COMMON} ${LIBS}

mesh.o: src/mesh.cpp
	${CXX} ${CFLAGS} -L/home/anl/PGK/lib/ -c src/mesh.cpp -o mesh.o  ${COMMON} ${LIBS}

model.o: src/model.cpp
	${CXX} ${CFLAGS} -Wno-implicit-fallthrough -L/home/anl/PGK/lib/ -c src/model.cpp -o model.o  ${COMMON} ${LIBS}

openglPreview.o: src/openglPreview.cpp
	${CXX} ${CFLAGS} -Wno-unused-parameter -L/home/anl/PGK/lib/ -c src/openglPreview.cpp -o openglPreview.o  ${COMMON} ${LIBS}

camera.o: src/camera.cpp
	${CXX} ${CFLAGS} -L/home/anl/PGK/lib/ -c src/camera.cpp -o camera.o  ${COMMON} ${LIBS}

rayCaster.o: src/rayCaster.cpp
	${CXX} ${CFLAGS} -L/home/anl/PGK/lib/ -c src/rayCaster.cpp -o rayCaster.o  ${COMMON} ${LIBS}

shader.o: src/shader.cpp
	${CXX} ${CFLAGS} -L/home/anl/PGK/lib/ -c src/shader.cpp -o shader.o  ${COMMON} ${LIBS}

clean:
	rm -f main *.o


