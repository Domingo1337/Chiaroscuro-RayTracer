# Przykladowy Makefile dla tutorialu06 dzialajacy na pracowni
# (c) anl 2015-2017 
#
CXX= g++ -fopenmp
CFLAGS= -I/usr/local/include/GLFW/ -I./include/  -std=c++14  -Wall -Wextra -O3
LIBS=-lglfw3 -lGL  -ldl -lX11 -lXxf86vm -lXrandr -lXi -lXinerama -lXcursor -lassimp -lfreeimage

default: main

.cpp.o:
	${CXX} -c ${CFLAGS} $<

main: main.o glad.o scene.o mesh.o model.o openglPreview.o camera.o rayTracer.o shader.o kdtree.o brdf.o prng.o
	${CXX} -Wall -Wextra main.o glad.o mesh.o scene.o model.o openglPreview.o camera.o rayTracer.o shader.o kdtree.o brdf.o prng.o -o main ${LIBS}

main.o: main.cpp
	${CXX} ${CFLAGS} -c main.cpp -o main.o  ${LIBS}

glad.o: src/glad.c
	${CXX} ${CFLAGS} -c src/glad.c -o glad.o ${LIBS}


scene.o: src/scene.cpp
	${CXX} ${CFLAGS} -c src/scene.cpp -o scene.o ${LIBS}

mesh.o: src/mesh.cpp
	${CXX} ${CFLAGS} -c src/mesh.cpp -o mesh.o ${LIBS}

model.o: src/model.cpp
	${CXX} ${CFLAGS} -Wno-implicit-fallthrough -c src/model.cpp -o model.o ${LIBS}

openglPreview.o: src/openglPreview.cpp
	${CXX} ${CFLAGS} -Wno-unused-parameter -c src/openglPreview.cpp -o openglPreview.o ${LIBS}

camera.o: src/camera.cpp
	${CXX} ${CFLAGS} -c src/camera.cpp -o camera.o ${LIBS}

rayTracer.o: src/rayTracer.cpp
	${CXX} ${CFLAGS} -c src/rayTracer.cpp -o rayTracer.o ${LIBS}

shader.o: src/shader.cpp
	${CXX} ${CFLAGS} -c src/shader.cpp -o shader.o ${LIBS}

kdtree.o: src/kdtree.cpp
	${CXX} ${CFLAGS} -c src/kdtree.cpp -o kdtree.o ${LIBS}

brdf.o: src/brdf.cpp
	${CXX} ${CFLAGS} -Wno-unused-parameter -c src/brdf.cpp -o brdf.o ${LIBS}

prng.o: src/prng.cpp
	${CXX} ${CFLAGS} -c src/prng.cpp -o prng.o ${LIBS}

clean:
	rm -f main *.o


