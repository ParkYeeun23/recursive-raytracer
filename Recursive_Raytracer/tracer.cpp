#include "tracer.h"

#include <math.h>
#include <float.h>

using namespace raytraceData;

tracer::tracer() : numSpheres(0), numCones(0), numCylinders(0), p1(NULL)
{
	for (int i = 0; i < MAX_SPHERES; i++) spheres[i] = NULL;
	for (int i = 0; i < MAX_CONES; i++) cones[i] = NULL;
	for (int i = 0; i < MAX_CYLINDERS; i++) cylinders[i] = NULL;
}

tracer::~tracer()
{
}

void tracer::findPointOnRay(ray* r, float t, point* p) {
	p->x = r->start->x + t * r->end->x;
	p->y = r->start->y + t * r->end->y;
	p->z = r->start->z + t * r->end->z;
	p->w = 1.0;
}

int tracer::raySphereIntersect(ray* r, sphere* s, float* t) {
	point p;
	float a, b, c, D;
	point* v;

	p.x = r->start->x - s->c->x;
	p.y = r->start->y - s->c->y;
	p.z = r->start->z - s->c->z;
	v = r->end;

	a = v->x * v->x + v->y * v->y + v->z * v->z;
	b = 2 * (v->x * p.x + v->y * p.y + v->z * p.z);
	c = p.x * p.x + p.y * p.y + p.z * p.z - s->r * s->r;
	D = b * b - 4 * a * c;

	if (D < 0) return FALSE;

	D = static_cast<float>(sqrt(D));
	*t = (-b - D) / (2 * a);
	if (*t < 1e-4f) *t = (-b + D) / (2 * a);
	if (*t < 1e-4f) return FALSE;
	return TRUE;
}

void tracer::findSphereNormal(sphere* s, point* p, vector* n) {
	n->x = (p->x - s->c->x) / s->r;
	n->y = (p->y - s->c->y) / s->r;
	n->z = (p->z - s->c->z) / s->r;
	n->w = 0.0;
}

int tracer::rayPlaneIntersect(ray* r, plane* pl, float* t) {
	float nDotDir = pl->normal.x * r->end->x
		+ pl->normal.y * r->end->y
		+ pl->normal.z * r->end->z;

	if (fabsf(nDotDir) < 1e-6f) return FALSE;

	float nDotStart = pl->normal.x * r->start->x
		+ pl->normal.y * r->start->y
		+ pl->normal.z * r->start->z;

	*t = (pl->d - nDotStart) / nDotDir;
	return (*t > 1e-4f) ? TRUE : FALSE;
}

static void normalize3(vector* n) {
	float len = sqrtf(n->x * n->x + n->y * n->y + n->z * n->z);
	if (len > 1e-8f) { n->x /= len; n->y /= len; n->z /= len; }
	n->w = 0.0f;
}

// lateral surface: (qx-ax)^2 + (qz-az)^2 = k^2 (qy-ay)^2, k = radius/height,
// valid for baseY <= qy <= apex.y; plus a base cap
int tracer::rayConeIntersect(ray* r, cone* co, float* t, vector* n) {
	float ox = r->start->x, oy = r->start->y, oz = r->start->z;
	float dx = r->end->x, dy = r->end->y, dz = r->end->z;

	float ax = co->apex.x, ay = co->apex.y, az = co->apex.z;
	float k = co->radius / co->height;
	float k2 = k * k;
	float baseY = ay - co->height;

	float px = ox - ax, py = oy - ay, pz = oz - az;

	float A = dx * dx + dz * dz - k2 * dy * dy;
	float B = 2.0f * (px * dx + pz * dz - k2 * py * dy);
	float C = px * px + pz * pz - k2 * py * py;

	float bestT = FLT_MAX;
	int found = FALSE;
	vector bestN; bestN.x = bestN.y = bestN.z = 0.0f; bestN.w = 0.0f;

	// lateral surface
	if (fabsf(A) > 1e-8f) {
		float D = B * B - 4.0f * A * C;
		if (D >= 0.0f) {
			D = sqrtf(D);
			float roots[2] = { (-B - D) / (2.0f * A), (-B + D) / (2.0f * A) };
			for (int i = 0; i < 2; i++) {
				float tc = roots[i];
				if (tc < 1e-4f) continue;
				float hy = oy + tc * dy;
				if (hy < baseY || hy > ay) continue;
				if (tc < bestT) {
					bestT = tc;
					float hx = ox + tc * dx, hz = oz + tc * dz;
					bestN.x = (hx - ax);
					bestN.y = -k2 * (hy - ay);
					bestN.z = (hz - az);
					normalize3(&bestN);
					found = TRUE;
				}
			}
		}
	}

	// base cap
	if (fabsf(dy) > 1e-8f) {
		float tc = (baseY - oy) / dy;
		if (tc > 1e-4f && tc < bestT) {
			float hx = ox + tc * dx, hz = oz + tc * dz;
			float ddx = hx - ax, ddz = hz - az;
			if (ddx * ddx + ddz * ddz <= co->radius * co->radius) {
				bestT = tc;
				bestN.x = 0.0f; bestN.y = -1.0f; bestN.z = 0.0f; bestN.w = 0.0f;
				found = TRUE;
			}
		}
	}

	if (!found) return FALSE;
	*t = bestT;
	*n = bestN;
	return TRUE;
}

// vertical (+y) axis, finite height with both caps
int tracer::rayCylinderIntersect(ray* r, cylinder* cy, float* t, vector* n) {
	float ox = r->start->x, oy = r->start->y, oz = r->start->z;
	float dx = r->end->x, dy = r->end->y, dz = r->end->z;

	float cx = cy->base.x, cyy = cy->base.y, cz = cy->base.z;
	float topY = cyy + cy->height;
	float R = cy->radius;

	float px = ox - cx, pz = oz - cz;

	float bestT = FLT_MAX;
	int found = FALSE;
	vector bestN; bestN.x = bestN.y = bestN.z = 0.0f; bestN.w = 0.0f;

	// lateral surface
	float A = dx * dx + dz * dz;
	if (fabsf(A) > 1e-8f) {
		float B = 2.0f * (px * dx + pz * dz);
		float C = px * px + pz * pz - R * R;
		float D = B * B - 4.0f * A * C;
		if (D >= 0.0f) {
			D = sqrtf(D);
			float roots[2] = { (-B - D) / (2.0f * A), (-B + D) / (2.0f * A) };
			for (int i = 0; i < 2; i++) {
				float tc = roots[i];
				if (tc < 1e-4f) continue;
				float hy = oy + tc * dy;
				if (hy < cyy || hy > topY) continue;
				if (tc < bestT) {
					bestT = tc;
					float hx = ox + tc * dx, hz = oz + tc * dz;
					bestN.x = hx - cx; bestN.y = 0.0f; bestN.z = hz - cz;
					normalize3(&bestN);
					found = TRUE;
				}
			}
		}
	}

	// caps (top: +y, bottom: -y)
	if (fabsf(dy) > 1e-8f) {
		float capY[2] = { cyy, topY };
		float capN[2] = { -1.0f, 1.0f };
		for (int i = 0; i < 2; i++) {
			float tc = (capY[i] - oy) / dy;
			if (tc < 1e-4f || tc >= bestT) continue;
			float hx = ox + tc * dx, hz = oz + tc * dz;
			float ddx = hx - cx, ddz = hz - cz;
			if (ddx * ddx + ddz * ddz <= R * R) {
				bestT = tc;
				bestN.x = 0.0f; bestN.y = capN[i]; bestN.z = 0.0f; bestN.w = 0.0f;
				found = TRUE;
			}
		}
	}

	if (!found) return FALSE;
	*t = bestT;
	*n = bestN;
	return TRUE;
}

// nearest hit across all primitives
void tracer::trace(ray* r, point* p, vector* n, material** m) {
	float tMin = FLT_MAX;
	int found = FALSE;
	vector bestN; bestN.x = bestN.y = bestN.z = 0.0f; bestN.w = 0.0f;
	material* bestM = NULL;

	for (int i = 0; i < numSpheres; i++) {
		float t;
		if (raySphereIntersect(r, spheres[i], &t) && t < tMin) {
			tMin = t;
			point hp; findPointOnRay(r, t, &hp);
			findSphereNormal(spheres[i], &hp, &bestN);
			bestM = spheres[i]->m;
			found = TRUE;
		}
	}

	if (p1) {
		float t;
		if (rayPlaneIntersect(r, p1, &t) && t < tMin) {
			tMin = t;
			bestN = p1->normal;
			bestM = p1->m;
			found = TRUE;
		}
	}

	for (int i = 0; i < numCones; i++) {
		float t; vector cn;
		if (rayConeIntersect(r, cones[i], &t, &cn) && t < tMin) {
			tMin = t;
			bestN = cn;
			bestM = cones[i]->m;
			found = TRUE;
		}
	}

	for (int i = 0; i < numCylinders; i++) {
		float t; vector cn;
		if (rayCylinderIntersect(r, cylinders[i], &t, &cn) && t < tMin) {
			tMin = t;
			bestN = cn;
			bestM = cylinders[i]->m;
			found = TRUE;
		}
	}

	if (found) {
		findPointOnRay(r, tMin, p);
		*n = bestN;
		*m = bestM;
	}
	else {
		p->w = 0.0f;
	}
}
