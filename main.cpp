#include <openglPreview.hpp>

/* TODO:
 *   Calculate normals if they are not given in obj file.
 */

int main(int argc, char *argv[]) {
    usingOpenGLPreview = true;
    Scene scene(argc > 1 ? argv[1] : "view_test.rtc");

    if (usingOpenGLPreview) {
        OpenGLPreview preview(&scene);
        Model model(scene.objFile);
        RayCaster renderer(model, scene);
        preview.setModel(&model);
        preview.setRenderer(&renderer);
        preview.loop();
        renderer.exportImage("render.png", "png");
    } else {
        Model model(scene.objFile);
        RayCaster renderer(model, scene);
        renderer.rayTrace(scene.VP, scene.LA, scene.UP, scene.yview);
        renderer.exportImage("xd.jpg", "jpg");
    }

    return 0;
}
