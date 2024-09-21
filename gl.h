#pragma once

#include <Eigen/Dense>
#include "tgaimage.h"
#include "shader.h"

typedef Eigen::Matrix4f Matrix;
typedef Eigen::Vector3f Vec3f;
typedef Eigen::Vector2f Vec2f;

Matrix getViewport(int w, int h);
Matrix getProjection(Vec3f camera, Vec3f center);
Matrix getView(Vec3f camera, Vec3f center, Vec3f up);
void line(int x0, int y0, int x1, int y1, TGAImage& image, TGAColor color);
void lineBresenham(int x0, int y0, int x1, int y1, TGAImage& image, TGAColor color);
Vec3f barycentric(Vec3f A, Vec3f B, Vec3f C, Vec3f P);
void triangleBoundingBox(Vec3f* verts, Shader& shader, TGAImage& image, float* zbuffer);