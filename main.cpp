#include "model.hpp"
#include "openglPreview.hpp"
#include "rayCaster.hpp"

/* TODO:
 *   Calculate normals if they are not given in obj file.
 */

int main(int argc, char *argv[]) {
    bool usingOpenGLPreview = true;
    unsigned int previewHeight = 900;

    Scene scene(argc > 1 ? argv[1] : "view_test.rtc");
    OpenGLPreview preview(&scene, previewHeight);
    Model model(scene.objFile);
    RayCaster renderer(model, scene);

    if (usingOpenGLPreview) {
        preview.setModel(&model);
        preview.setRenderer(&renderer);
        preview.loop();
    } else {
        renderer.rayTrace(scene.VP, scene.LA, scene.UP, scene.yview);
    }

    renderer.exportImage("render.png", "png");
    return 0;
}
