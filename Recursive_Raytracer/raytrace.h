#pragma once

#include <glad/glad.h>
#include "raytraceData.h"
#include "lowlevel.h"
#include "shader.h"
#include "tracer.h"

#define MAX_DEPTH 5
#define MAX_LIGHTS 4
#define SHADOW_EPSILON 1e-3f

class raytrace {
public:
	raytrace(int w, int h);
	~raytrace();

	GLubyte* display(void);

private:
	void initScene();
	void initCamera(int w, int h);
	void drawScene(void);
	void stepPhysics();
	raytraceData::point* makePoint(GLfloat x, GLfloat y, GLfloat z, GLfloat w);
	raytraceData::sphere* makeSphere(GLfloat x, GLfloat y, GLfloat z, GLfloat r);
	bool shadowed(raytraceData::point* p, raytraceData::vector* n, raytraceData::lightSource* light);
	void rayColor(raytraceData::ray* r, raytraceData::color* c, int depth);
	void calculateDirection(raytraceData::point* p, raytraceData::point* q, raytraceData::point* v);

	int width;
	int height;

	raytraceData::point* viewpoint;
	GLfloat pnear;
	GLfloat fovx;

	raytraceData::lightSource lights[MAX_LIGHTS];
	int numLights;

	// per-sphere velocities for the bouncing balls
	GLfloat vx[MAX_SPHERES];
	GLfloat vy[MAX_SPHERES];
	GLfloat vz[MAX_SPHERES];
	GLfloat groundY;

	lowlevel lowlevel;
	shader shader;
	tracer tracer;
};
