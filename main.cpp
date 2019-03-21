#include <glad/glad.h>

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/intersect.hpp>

#include <shader.hpp>

#include <camera.hpp>
#include <model.hpp>
#include <rayCaster.hpp>
#include <scene.hpp>

#include <openglPreview.hpp>

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

/* TODO:
 *   Calculate normals if they are not given in obj file.
 */

int main(int argc, char *argv[]) {
    usingOpenGLPreview = true;
    Scene scene(argc > 1 ? argv[1] : "view_test.rtc");

    if (usingOpenGLPreview) {
        OpenGLPreview preview(&scene);
        Model ourModel(scene.objFile);
        RayCaster renderer(ourModel, scene);
        preview.setModel(&ourModel);
        rendererPtr = &renderer;
        preview.loop(renderer.getData(), scene.xres, scene.yres);
        renderer.exportImage("render.png", "png");
    } else {
        Model ourModel(scene.objFile);
        RayCaster renderer(ourModel, scene);
        renderer.rayTrace(scene.VP, scene.LA, scene.UP, scene.yview);
        renderer.exportImage("xd.jpg", "jpg");
    }

    return 0;
}
