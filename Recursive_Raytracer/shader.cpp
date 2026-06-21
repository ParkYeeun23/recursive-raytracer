#include "shader.h"

#include <math.h>

using namespace raytraceData;

shader::shader()
{
}

shader::~shader()
{
}

material* shader::makeMaterial(
	GLfloat r, GLfloat g, GLfloat b,
	GLfloat amb, GLfloat diff, GLfloat spec, GLfloat shine,
	GLfloat refl, GLfloat refr, GLfloat ior,
	int metal, int checkerboard)
{
	material* m = new material();
	m->c.r = r; m->c.g = g; m->c.b = b;
	m->amb = amb;
	m->diff = diff;
	m->spec = spec;
	m->shine = shine;
	m->refl = refl;
	m->refr = refr;
	m->ior = ior;
	m->metal = metal;
	m->checkerboard = checkerboard;
	return m;
}

color shader::surfaceColor(point* p, material* m) {
	if (m->checkerboard) {
		int cx = (int)floorf(p->x * 1.0f);
		int cz = (int)floorf(p->z * 1.0f);
		if (((cx + cz) & 1) == 0) return { 0.9f, 0.9f, 0.9f };
		return { 0.1f, 0.1f, 0.1f };
	}
	return m->c;
}

void shader::shadeAmbient(point* p, material* m, color* c) {
	color surf = surfaceColor(p, m);
	c->r = m->amb * surf.r;
	c->g = m->amb * surf.g;
	c->b = m->amb * surf.b;
}

void shader::addLight(point* p, vector* n, material* m,
	vector* viewDir, lightSource* light, bool inShadow, color* c)
{
	if (inShadow) return;

	// direction to the light
	float lx, ly, lz;
	if (light->type == 1) {
		lx = -light->dir.x; ly = -light->dir.y; lz = -light->dir.z;
	}
	else {
		lx = light->pos.x - p->x;
		ly = light->pos.y - p->y;
		lz = light->pos.z - p->z;
	}
	float lLen = sqrtf(lx * lx + ly * ly + lz * lz);
	if (lLen < 1e-8f) return;
	lx /= lLen; ly /= lLen; lz /= lLen;

	color surf = surfaceColor(p, m);

	// diffuse
	float nDotL = n->x * lx + n->y * ly + n->z * lz;
	if (nDotL < 0.0f) nDotL = 0.0f;

	c->r += m->diff * nDotL * surf.r * light->intensity.r;
	c->g += m->diff * nDotL * surf.g * light->intensity.g;
	c->b += m->diff * nDotL * surf.b * light->intensity.b;

	// specular (Blinn-Phong)
	if (nDotL > 0.0f) {
		float hx = lx + viewDir->x;
		float hy = ly + viewDir->y;
		float hz = lz + viewDir->z;
		float hLen = sqrtf(hx * hx + hy * hy + hz * hz);
		if (hLen > 1e-6f) {
			hx /= hLen; hy /= hLen; hz /= hLen;
			float nDotH = n->x * hx + n->y * hy + n->z * hz;
			if (nDotH < 0.0f) nDotH = 0.0f;
			float specFactor = powf(nDotH, m->shine);
			c->r += m->spec * specFactor * light->intensity.r;
			c->g += m->spec * specFactor * light->intensity.g;
			c->b += m->spec * specFactor * light->intensity.b;
		}
	}
}
