#include "gl.h"

Matrix getViewport(int w, int h) {
    Matrix m = Matrix::Identity();
    m(0, 3) = w / 2.0;
    m(1, 3) = h / 2.0;
    m(0, 0) = w / 2.0;
    m(1, 1) = h / 2.0;
    return m;
}

Matrix getProjection(Vec3f camera, Vec3f center) {
    Matrix Projection = Matrix::Identity();
    Projection(3, 2) = -1 / (camera - center).norm();
    return Projection;
}

Matrix getView(Vec3f camera, Vec3f center, Vec3f up) {
    Vec3f z = camera - center;
    Vec3f x = up.cross(z);
    Vec3f y = z.cross(x);
    x.normalize(); y.normalize(); z.normalize();
    Matrix r = Matrix::Identity();
    for (int i = 0; i < 3; i++) {
        r(0, i) = x[i];
        r(1, i) = y[i];
        r(2, i) = z[i];
    }
    Matrix t = Matrix::Identity();
    for (int i = 0; i < 3; i++) {
        t(i, 3) = -camera[i];
    }
    Matrix res = r * t;
    return res;
}

void line(int x0, int y0, int x1, int y1, TGAImage& image, TGAColor color) {
    bool f = true;
    if (abs(x1 - x0) < abs(y1 - y0)) {
        std::swap(x0, y0);
        std::swap(x1, y1);
        f = false;
    }
    if (x0 > x1) {
        std::swap(x0, x1);
        std::swap(y0, y1);
    }
    for (int x = x0; x <= x1; x++) {
        float t = 1.0 * (x - x0) / (x1 - x0);
        int y = static_cast<int>(y0 + (y1 - y0) * t);
        if (f)
            image.set(x, y, color);
        else
            image.set(y, x, color);
    }
}

void lineBresenham(int x0, int y0, int x1, int y1, TGAImage& image, TGAColor color) {
    bool f = true;
    if (abs(x1 - x0) < abs(y1 - y0)) {
        std::swap(x0, y0);
        std::swap(x1, y1);
    }
    if (x1 < x0) {
        std::swap(x0, x1);
        std::swap(y0, y1);
    }
    int y = y0;
    int d = 0;
    int dx = x1 - x0, dy = y1 - y0;
    for (int x = x0; x <= x1; x++) {
        if (f) {
            image.set(x, y, color);
        }
        else {
            image.set(y, x, color);
        }
        d += dy * 2;
        if (d - dx > 0) {
            y += (y1 > y0 ? 1 : -1);
            d -= dx * 2;
        }
    }
}

Vec3f barycentric(Vec3f A, Vec3f B, Vec3f C, Vec3f P) {
    Vec3f q[2];
    for (int i = 0; i < 2; i++) {
        q[i][0] = B[i] - A[i];
        q[i][1] = C[i] - A[i];
        q[i][2] = A[i] - P[i];
    }
    Vec3f w = q[0].cross(q[1]);
    if (w[2] > 1e-2) {
        float u = w[0] / w[2], v = w[1] / w[2];
        return Vec3f(1.0f - u - v, u, v);
    }
    return Vec3f(-1, 1, 1);
}

void triangleBoundingBox(Vec3f* verts, Shader& shader, TGAImage& image, float* zbuffer) {
    Vec2f bboxmin(image.get_width(), image.get_height());
    Vec2f bboxmax(0.0f, 0.0f);
    for (int i = 0; i < 3; i++) {
        bboxmin.x() = std::max(0.0f, std::min(verts[i].x(), bboxmin.x()));
        bboxmin.y() = std::max(0.0f, std::min(verts[i].y(), bboxmin.y()));
        bboxmax.x() = std::min(1.0f * image.get_width(), std::max(verts[i].x(), bboxmax.x()));
        bboxmax.y() = std::min(1.0f * image.get_height(), std::max(verts[i].y(), bboxmax.y()));
    }
    for (int x = bboxmin.x(); x <= bboxmax.x(); x++) {
        for (int y = bboxmin.y(); y <= bboxmax.y(); y++) {
            Vec3f P(x, y, 0.0f);
            Vec3f bc = barycentric(verts[0], verts[1], verts[2], P);
            if (bc[0] < 0 || bc[1] < 0 || bc[2] < 0) continue;
            for (int i = 0; i < 3; i++) {
                P.z() += verts[i].z() * bc[i];
            }
            int width = image.get_width();
            if (zbuffer[(int)(P.x() + P.y() * width)] < P.z()) {
                zbuffer[(int)(P.x() + P.y() * width)] = P.z();
                TGAColor color;
                bool discard = shader.fragment(bc, color);
                if (!discard)
                    image.set(P.x(), P.y(), color);
                else
                    image.set(P.x(), P.y(), TGAColor(0, 0, 0));
            }
        }
    }
}