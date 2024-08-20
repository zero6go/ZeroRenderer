#ifndef __MODEL_H__
#define __MODEL_H__

#include <vector>
#include "geometry.h"
#include "tgaimage.h"

class Model {
private:
	std::vector<Vec3f> verts_;
	std::vector<std::vector<Vec3i> > faces_; // attention, this Vec3i means vertex/uv/normal
	std::vector<Vec3f> norms_;
	std::vector<Vec2f> uv_;
	TGAImage diffusemap_;
	TGAImage normalmap_;
	TGAImage specularmap_;
	bool active;
	void load_texture(std::string filename, const char* suffix, TGAImage& img);
public:
	Model(std::string filename);
	~Model();
	bool isActive();
	int nverts();
	int nfaces();
	Vec3f vert(int idxface, int idxvert);
	Vec2i uv(int idxface, int idxvert);
	Vec3f normal(int idxface, int idxvert);
	TGAImage getTexture();
	TGAImage getSpecular();
	TGAImage getNormal();
};

#endif //__MODEL_H__