#pragma once

#include <glad/glad.h>
#include <stdlib.h>
#include "raytraceData.h"

class shader {
public:
	shader();
	~shader();

	raytraceData::material* makeMaterial(
		GLfloat r, GLfloat g, GLfloat b,
		GLfloat amb, GLfloat diff, GLfloat spec, GLfloat shine,
		GLfloat refl, GLfloat refr, GLfloat ior,
		int metal = 0, int checkerboard = 0
	);

	raytraceData::color surfaceColor(raytraceData::point* p, raytraceData::material* m);
	void shadeAmbient(raytraceData::point* p, raytraceData::material* m, raytraceData::color* c);

	// diffuse + specular for one light
	void addLight(
		raytraceData::point* p,
		raytraceData::vector* n,
		raytraceData::material* m,
		raytraceData::vector* viewDir,
		raytraceData::lightSource* light,
		bool inShadow,
		raytraceData::color* c
	);
};
