#pragma once

#include "raytraceData.h"

#define MAX_SPHERES 8
#define MAX_CONES 4
#define MAX_CYLINDERS 4

class tracer {
public:
	tracer();
	~tracer();

	void findPointOnRay(raytraceData::ray* r, float t, raytraceData::point* p);

	int raySphereIntersect(raytraceData::ray* r, raytraceData::sphere* s, float* t);
	void findSphereNormal(raytraceData::sphere* s, raytraceData::point* p, raytraceData::vector* n);

	int rayPlaneIntersect(raytraceData::ray* r, raytraceData::plane* pl, float* t);

	int rayConeIntersect(raytraceData::ray* r, raytraceData::cone* co, float* t, raytraceData::vector* n);
	int rayCylinderIntersect(raytraceData::ray* r, raytraceData::cylinder* cy, float* t, raytraceData::vector* n);

	void trace(raytraceData::ray* r, raytraceData::point* p, raytraceData::vector* n, raytraceData::material** m);

	raytraceData::sphere* spheres[MAX_SPHERES];
	int numSpheres;
	raytraceData::cone* cones[MAX_CONES];
	int numCones;
	raytraceData::cylinder* cylinders[MAX_CYLINDERS];
	int numCylinders;
	raytraceData::plane* p1;
};
