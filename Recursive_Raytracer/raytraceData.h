#pragma once

#include <glad/glad.h>

#define TRUE 1
#define FALSE 0
#define M_PI 3.1415926535897932384626433832795029

namespace raytraceData {

	typedef struct point {
		GLfloat x;
		GLfloat y;
		GLfloat z;
		GLfloat w;
	} point;

	typedef point vector;

	typedef struct segment {
		point* start;
		point* end;
	} segment;

	typedef segment ray;

	typedef struct color {
		GLfloat r;
		GLfloat g;
		GLfloat b;
	} color;

	typedef struct material {
		color c;
		GLfloat amb;      // ambient coefficient
		GLfloat diff;     // diffuse coefficient
		GLfloat spec;     // specular strength
		GLfloat shine;    // specular exponent
		GLfloat refl;     // reflectivity [0,1]
		GLfloat refr;     // refractivity [0,1]
		GLfloat ior;      // index of refraction
		int metal;        // 1 = tint reflections by surface color
		int checkerboard; // 1 = checkerboard pattern
	} material;

	typedef struct sphere {
		point* c;   // center
		GLfloat r;  // radius
		material* m;
	} sphere;

	// plane: normal . p = d
	typedef struct plane {
		vector normal;
		GLfloat d;
		material* m;
	} plane;

	// finite cone, vertical (+y) axis, apex at the top
	typedef struct cone {
		point apex;
		GLfloat radius; // base radius
		GLfloat height; // apex.y - base.y
		material* m;
	} cone;

	// finite cylinder, vertical (+y) axis
	typedef struct cylinder {
		point base;     // center of the bottom cap
		GLfloat radius;
		GLfloat height; // extends upward from base.y
		material* m;
	} cylinder;

	// type 0 = point light (uses pos), type 1 = directional (uses dir)
	typedef struct lightSource {
		int type;
		point pos;
		vector dir;     // travel direction for a directional light
		color intensity;
	} lightSource;

}
