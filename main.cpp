#include "tgaimage.h"
#include "model.h"
#include <iostream>
#include "gl.h"
#include "shader.h"

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);
const TGAColor green = TGAColor(0, 255, 0, 255);
const int width = 2000;
const int height = 2000;
Model* model = nullptr;
Vec3f lightDir(0.3, -0.7, -1);
Vec3f camera(0.25, 0.3, 2);
Vec3f center(0, 0, 0);
Vec3f viewDir = center - camera;
float ambient = 0.1f;

int main(int argc, char** argv) {
    TGAImage image(width, height, TGAImage::RGB);
    Matrix shadowMVP;
    float* zbuffer = new float[width * height];
    for (int i = 0; i < width * height; i++) {
        zbuffer[i] = -std::numeric_limits<float>::max();
    }
    std::string s;
    //obj/african_head/african_head.obj
    //obj/african_head/african_head_eye_inner.obj
    while(std::cin >> s){
        model = new Model(s);
        if (!model->isActive()) break;
        Matrix viewport = getViewport(width, height);
        Matrix projection = getProjection(camera, center);
        Matrix view = getView(camera, center, Vec3f(0, 1.0f, 0));

        TGAImage texture = model->getTexture();
        TGAImage specularMap = model->getSpecular();
        TGAImage normalMap = model->getNormal();

        //FlatShader shader(viewport, projection, view, lightDir, texture);
        //GouraudShader shader(viewport, projection, view, lightDir, texture);
        //ToonShader shader(viewport, projection, view, lightDir);
        //PhongShader shader(viewport, projection, view, lightDir, texture, ambient, viewDir, specularMap, 64.0f, normalMap);
        BlinnPhongShader shader(viewport, projection, view, lightDir, texture, ambient, viewDir, specularMap, 64.0f, normalMap);
        for (int i = 0; i < model->nfaces(); i++) {
            Vec3f screenCoords[3];
            for (int j = 0; j < 3; j++) {
                Vec3f v = model->vert(i, j);
                Vec2i uv = model->uv(i, j);
                Vec3f normal = model->normal(i, j);
                screenCoords[j] = shader.vertex(v, uv, normal, j);
            }
            triangleBoundingBox(screenCoords, shader, image, zbuffer);
        }
        std::cout << "Completed!" << std::endl;
    }

    image.flip_vertically();
    image.write_tga_file("output.tga");

    delete model;
    delete[] zbuffer;
    return 0;
}
