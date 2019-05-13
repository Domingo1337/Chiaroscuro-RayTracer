#include "model.hpp"
#include "openglPreview.hpp"
#include "rayTracer.hpp"

#include <iostream>

/* TODO:
 *   Calculate normals if they are not given in obj file.
 */

int main(int argc, char **argv) {
    Scene scene(argc, argv);
    OpenGLPreview preview(&scene);
    Model model(scene);
    RayTracer renderer(model, scene);

    if (scene.usingOpenGLPreview) {
        preview.setModel(&model);
        preview.setRenderer(&renderer);
        preview.loop();
    } else {
        renderer.rayTrace(scene.VP, scene.LA, scene.UP, scene.yview);
        renderer.normalizeImage();
    }

    renderer.exportImage(scene.renderPath.c_str());
    return 0;
}
