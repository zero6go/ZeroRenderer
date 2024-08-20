#pragma once

#include "geometry.h"
#include "tgaimage.h"

class Shader {
public:
    //���㲢����MVP�任��Ķ�������Ļ�ϵ����꣬ͬʱ����ƬԪ��ɫ�����������
	virtual Vec3f vertex(Vec3f modelVertex, Vec2i uv, Vec3f normal, int idx) = 0;
    //����ƬԪ��ɫ���ж��Ƿ���Ҫ��Ⱦ
	virtual bool fragment(Vec3f bc, TGAColor& color) = 0;

	Vec3f mvp(Matrix Viewport, Matrix Projection, Matrix View, Vec3f ModelVertex) {
		mat<4, 1, float> matv;
		matv[3][0] = 1.0f;
		for (int i = 0; i < 3; i++) matv[i][0] = ModelVertex[i];
		mat<4, 1, float> m = Viewport * Projection * View * matv;
		return Vec3f(m[0][0] / m[3][0], m[1][0] / m[3][0], m[2][0] / m[3][0]);
	}
    //����TBN�����
    mat<3, 3, float> tbn(Vec3f* v, Vec2i* uv, Vec3f n) {
        Vec3f AB = v[1] - v[0];
        Vec3f AC = v[2] - v[0];

        Vec3f uv1 = Vec3f(uv[1].x - uv[0].x, uv[1].y - uv[0].y, 0);
        Vec3f uv2 = Vec3f(uv[2].x - uv[0].x, uv[2].y - uv[0].y, 0);

        Vec3f T = (AB * uv2[1] - AC * uv1[1]) / (uv1[0] * uv2[1] - uv2[0] * uv1[1]);
        Vec3f B = (AC * uv1[0] - AB * uv2[0]) / (uv1[0] * uv2[1] - uv2[0] * uv1[1]);
        Vec3f q, w;
        for (int i = 0; i < 3; i++) {
            q[i] = (T * n) * n[i];
        }
        Vec3f t = T - q;
        t.normalize();
        for (int i = 0; i < 3; i++) {
            q[i] = (B * n) * n[i];
            w[i] = (B * t) * t[i];
        }
        Vec3f b = B - q - w;
        b.normalize();
        mat<3, 3, float> TBN;
        for (int i = 0; i < 3; i++) {
            TBN[i][0] = t[i];
            TBN[i][1] = b[i];
            TBN[i][2] = n[i];
        }
        return TBN;
    }
};

class FlatShader :public Shader {
private:
    Matrix viewport;
    Matrix projection;
    Matrix view;
    Vec3f v[3];
    Vec2i uv[3];
    Vec3f lightDir;
    TGAImage texture;
public:
    FlatShader(Matrix viewport, Matrix projection, Matrix view, Vec3f lightDir, TGAImage& texture) {
        this->viewport = viewport;
        this->projection = projection;
        this->view = view;
        this->lightDir = lightDir.normalize();
        this->texture = texture;
    }

    Vec3f vertex(Vec3f modelVertex, Vec2i uv, Vec3f normal, int idx) {
        this->uv[idx] = uv;
        this->v[idx] = modelVertex;

        Vec3f gl_Pos;
        gl_Pos = mvp(viewport, projection, view, modelVertex);
        return gl_Pos;
    }
    bool fragment(Vec3f bc, TGAColor& color) {
        Vec3f normal = cross((v[1] - v[0]), (v[2] - v[0]));
        normal.normalize();
        Vec2i uvP;
        float intensityP = -(lightDir * normal);
        for (int i = 0; i < 3; i++) {
            uvP.x += uv[i].x * bc[i];
            uvP.y += uv[i].y * bc[i];
        }
        for (int i = 0; i < 3; i++) color[i] = std::min(255.0f, texture.get(uvP.x, uvP.y)[i] * intensityP);
        return false ? intensityP > 0:intensityP <= 0;
    }
};

class GouraudShader :public Shader {
private:
    Matrix viewport;
    Matrix projection;
    Matrix view;
    Vec3f normal[3];
    Vec2i uv[3];
    Vec3f lightDir;
    TGAImage texture;
public:
    GouraudShader(Matrix viewport, Matrix projection, Matrix view, Vec3f lightDir, TGAImage& texture) {
        this->viewport = viewport;
        this->projection = projection;
        this->view = view;
        this->lightDir = lightDir.normalize();
        this->texture = texture;
    }

    Vec3f vertex(Vec3f modelVertex, Vec2i uv, Vec3f normal, int idx) {
        this->uv[idx] = uv;
        this->normal[idx] = normal;

        Vec3f gl_Pos;
        gl_Pos = mvp(viewport, projection, view, modelVertex);
        return gl_Pos;
    }
    bool fragment(Vec3f bc, TGAColor& color) {
        float intensity[3];
        for (int i = 0; i < 3; i++) {
            intensity[i] = -(lightDir * normal[i]);
        }
        Vec2i uvP;
        float intensityP = 0.0f;
        for (int i = 0; i < 3; i++) {
            uvP.x += uv[i].x * bc[i];
            uvP.y += uv[i].y * bc[i];
            intensityP += intensity[i] * bc[i];
        }
        for (int i = 0; i < 3; i++) color[i] = std::min(255.0f, texture.get(uvP.x, uvP.y)[i] * intensityP);
        return false ? intensityP > 0:intensityP <= 0;
    }
};

class ToonShader :public Shader {
private:
    Matrix viewport;
    Matrix projection;
    Matrix view;
    Vec3f normal[3];
    Vec2i uv[3];
    Vec3f lightDir;
public:
    ToonShader(Matrix viewport, Matrix projection, Matrix view, Vec3f lightDir) {
        this->viewport = viewport;
        this->projection = projection;
        this->view = view;
        this->lightDir = lightDir.normalize();
    }

    Vec3f vertex(Vec3f modelVertex, Vec2i uv, Vec3f normal, int idx) {
        this->uv[idx] = uv;
        this->normal[idx] = normal;

        Vec3f gl_Pos;
        gl_Pos = mvp(viewport, projection, view, modelVertex);
        return gl_Pos;
    }
    bool fragment(Vec3f bc, TGAColor& color) {
        float intensity[3];
        float intensityP = 0;
        for (int i = 0; i < 3; i++) {
            intensity[i] = -(normal[i] * lightDir);
            intensityP += intensity[i] * bc[i];
        }
        if (intensityP > .85) intensityP = 1;
        else if (intensityP > .60) intensityP = .80;
        else if (intensityP > .45) intensityP = .60;
        else if (intensityP > .30) intensityP = .45;
        else if (intensityP > .15) intensityP = .30;
        color = TGAColor(255, 105, 180) * intensityP;
        return false ? intensityP > 0:intensityP <= 0;
    }
};

class PhongShader :public Shader {
private:
    Matrix viewport;
    Matrix projection;
    Matrix view;
    Vec3f normal[3];
    Vec2i uv[3];
    Vec3f lightDir;
    TGAImage texture;

    float ambient;
    Vec3f viewDir;
    TGAImage specularMap;
    float shininess;
    TGAImage normalMap;
    Vec3f v[3];
public:
    PhongShader(Matrix viewport, Matrix projection, Matrix view, Vec3f lightDir, TGAImage& texture, float ambient, Vec3f viewDir, TGAImage& specularMap, float shininess, TGAImage& normalMap) {
        this->viewport = viewport;
        this->projection = projection;
        this->view = view;
        this->lightDir = lightDir.normalize();
        this->texture = texture;
        this->ambient = ambient;
        this->viewDir = viewDir.normalize();
        this->specularMap = specularMap;
        this->shininess = shininess;
        this->normalMap = normalMap;
    }

    Vec3f vertex(Vec3f modelVertex, Vec2i uv, Vec3f normal, int idx) {
        this->uv[idx] = uv;
        this->normal[idx] = normal;
        this->v[idx] = modelVertex;

        Vec3f gl_Pos;
        gl_Pos = mvp(viewport, projection, view, modelVertex);
        return gl_Pos;
    }
    bool fragment(Vec3f bc, TGAColor& color) {
        Vec3f normalP;
        Vec2i uvP;
        Vec3f n;
        for (int i = 0; i < 3; i++) {
            uvP.x += uv[i].x * bc[i];
            uvP.y += uv[i].y * bc[i];

            //��ֵ��õ�ǰ���ط�����
            n.x += normal[i].x * bc[i];
            n.y += normal[i].y * bc[i];
            n.z += normal[i].z * bc[i];
        }
        mat<3, 3, float> TBN = tbn(v, uv, n);
        //������ͼrgb�ֱ𱣴淨������xyz
        TGAColor c = normalMap.get(uvP[0], uvP[1]);
        for (int i = 0; i < 3; i++)
            normalP[2 - i] = (float)c[i] / 255.f * 2.f - 1.f;
        normalP.normalize();
        //TBN�����ʹ���߿ռ䷨����ת��������ռ���
        normalP = TBN * normalP;
        normalP.normalize();

        float diffuse = -(normalP * lightDir);
        if (diffuse < 0) return true;

        Vec3f reflectDir =normalP * (normalP * lightDir * 2) - lightDir;
        reflectDir.normalize();
        float specular = 0.6 * pow(std::max(0.0f, reflectDir * viewDir), shininess);
        for (int i = 0; i < 3; i++)
            color[i] = std::min(255.0f, texture.get(uvP.x, uvP.y)[i] * (ambient + diffuse) + specularMap.get(uvP.x, uvP.y)[i] * specular);
        return false;
    }
};

class BlinnPhongShader :public Shader {
private:
    Matrix viewport;
    Matrix projection;
    Matrix view;
    Vec3f normal[3];
    Vec2i uv[3];
    Vec3f lightDir;
    TGAImage texture;

    float ambient;
    Vec3f viewDir;
    TGAImage specularMap;
    float shininess;
    TGAImage normalMap;
    Vec3f v[3];
public:
    BlinnPhongShader(Matrix viewport, Matrix projection, Matrix view, Vec3f lightDir, TGAImage& texture, float ambient, Vec3f viewDir, TGAImage& specularMap, float shininess, TGAImage& normalMap) {
        this->viewport = viewport;
        this->projection = projection;
        this->view = view;
        this->lightDir = lightDir.normalize();
        this->texture = texture;
        this->ambient = ambient;
        this->viewDir = viewDir.normalize();
        this->specularMap = specularMap;
        this->shininess = shininess;
        this->normalMap = normalMap;
    }

    Vec3f vertex(Vec3f modelVertex, Vec2i uv, Vec3f normal, int idx) {
        this->uv[idx] = uv;
        this->normal[idx] = normal;
        this->v[idx] = modelVertex;

        Vec3f gl_Pos;
        gl_Pos = mvp(viewport, projection, view, modelVertex);
        return gl_Pos;
    }
    bool fragment(Vec3f bc, TGAColor& color) {
        Vec3f normalP;
        Vec2i uvP;
        Vec3f n;
        for (int i = 0; i < 3; i++) {
            uvP.x += uv[i].x * bc[i];
            uvP.y += uv[i].y * bc[i];

            //��ֵ��õ�ǰ���ط�����
            n.x += normal[i].x * bc[i];
            n.y += normal[i].y * bc[i];
            n.z += normal[i].z * bc[i];
        }
        
        mat<3, 3, float> TBN = tbn(v, uv, n);
        
        //������ͼrgb�ֱ𱣴淨������xyz
        TGAColor c = normalMap.get(uvP[0], uvP[1]);
        for (int i = 0; i < 3; i++)
            n[2 - i] = (float)c[i] / 255.f * 2.f - 1.f;
        n.normalize();
        //TBN�����ʹ���߿ռ䷨����ת��������ռ���
        normalP = TBN * n;
        normalP.normalize();

        float diffuse = -(normalP * lightDir);
        if (diffuse < 0) return true;

        //����view��light�н�һ��ķ�������������Phongģ�ͷ���������߼нǴ���90�ȵ��¸߹ⲻ���������
        Vec3f half = (lightDir + viewDir).normalize();
        float specular = 0.5 * pow(std::max(0.0f, -(half * normalP)), shininess);
        for (int i = 0; i < 4; i++)
            color[i] = std::min(255.0f, texture.get(uvP.x, uvP.y)[i] * (ambient + diffuse) + specularMap.get(uvP.x, uvP.y)[i] * specular);
        return false;
    }
};