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
 *   Decent image export.
 *   Move OpenGl to some class to have some nice abstraction.
 */

int main(int argc, char *argv[]) {
    usingOpenGLPreview = true;
    Scene scene(argc > 1 ? argv[1] : "view_test.rtc");

    OpenGLPreview *preview;

    if (usingOpenGLPreview) {
        preview = new OpenGLPreview(&scene);
    }

    Model ourModel(scene.objFile);

    if (usingOpenGLPreview) {
        preview->setModel(&ourModel);
        preview->loop();
        delete preview;
    }

    RayCaster renderer(ourModel, scene);
    renderer.rayTrace(camera.Position, camera.Front, camera.Up);
    renderer.printPPM();

    return 0;
}
