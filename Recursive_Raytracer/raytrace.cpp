#include "raytrace.h"
#include <math.h>
#include <cstdlib>

using namespace raytraceData;

raytrace::raytrace(int w, int h)
{
	lowlevel.initCanvas(w, h);
	initCamera(w, h);
	initScene();
}

raytrace::~raytrace()
{
	if (viewpoint != NULL) delete viewpoint;

	for (int i = 0; i < tracer.numSpheres; i++) {
		if (tracer.spheres[i]) {
			if (tracer.spheres[i]->c) delete tracer.spheres[i]->c;
			if (tracer.spheres[i]->m) delete tracer.spheres[i]->m;
			delete tracer.spheres[i];
		}
	}
	for (int i = 0; i < tracer.numCones; i++) {
		if (tracer.cones[i]) {
			if (tracer.cones[i]->m) delete tracer.cones[i]->m;
			delete tracer.cones[i];
		}
	}
	for (int i = 0; i < tracer.numCylinders; i++) {
		if (tracer.cylinders[i]) {
			if (tracer.cylinders[i]->m) delete tracer.cylinders[i]->m;
			delete tracer.cylinders[i];
		}
	}
	if (tracer.p1) {
		if (tracer.p1->m) delete tracer.p1->m;
		delete tracer.p1;
	}
}

GLubyte* raytrace::display(void)
{
	stepPhysics();
	drawScene();
	return lowlevel.flushCanvas();
}

void raytrace::initScene()
{
	groundY = -1.0f;

	// pink transparent sphere
	{
		GLfloat r = 0.6f;
		sphere* s = makeSphere(-1.1f, groundY + r, -5.4f, r);
		s->m = shader.makeMaterial(
			1.0f, 0.05f, 0.1f,
			0.00f, 1.0f, 1.9f, 180.0f,
			0.12f, 0.5f, 1.3f);
		tracer.spheres[tracer.numSpheres++] = s;
	}

	// clear transparent sphere
	{
		GLfloat r = 0.6f;
		sphere* s = makeSphere(0.9f, groundY + r, -6.0f, r);
		s->m = shader.makeMaterial(
			0.92f, 0.96f, 1.0f,
			0.04f, 0.06f, 10.95f, 220.0f,
			0.10f, 0.90f, 1.7f);
		tracer.spheres[tracer.numSpheres++] = s;
	}

	// gold chrome sphere
	{
		GLfloat r = 0.6f;
		sphere* s = makeSphere(-0.2f, groundY + r, -7.6f, r);
		s->m = shader.makeMaterial(
			1.0f, 0.78f, 0.34f,
			0.10f, 0.20f, 1.0f, 250.0f,
			0.85f, 0.0f, 1.0f,
			1);
		tracer.spheres[tracer.numSpheres++] = s;
	}

	// Lambert cone
	{
		cone* co = new cone();
		co->radius = 0.6f;
		co->height = 1.4f;
		co->apex.x = 2.8f;
		co->apex.y = groundY + co->height;   
		co->apex.z = -7.0f;
		co->apex.w = 1.0f;
		co->m = shader.makeMaterial(
			0.5f, 0.85f, 0.45f,
			0.15f, 0.85f, 0.6f, 32.0f,
			0.06f, 0.0f, 1.0f);
		tracer.cones[tracer.numCones++] = co;
	}

	// Lambert cylinder
	{
		cylinder* cy = new cylinder();
		cy->radius = 0.5f;
		cy->height = 1.3f;
		cy->base.x = -2.5f;
		cy->base.y = groundY;                
		cy->base.z = -6.7f;
		cy->base.w = 1.0f;
		cy->m = shader.makeMaterial(
			0.2f, 0.2f, 1.0f,
			0.2f, 1.0f, 0.65f, 132.0f,
			0.06f, 0.0f, 1.0f);
		tracer.cylinders[tracer.numCylinders++] = cy;
	}


	plane* p = new plane();
	p->normal.x = 0.0f; p->normal.y = 1.0f; p->normal.z = 0.0f; p->normal.w = 0.0f;
	p->d = groundY;
	p->m = shader.makeMaterial(
		0.8f, 0.8f, 0.8f,
		0.20f, 0.85f, 0.20f, 16.0f,
		0.12f, 0.0f, 1.0f,
		0, 1);
	tracer.p1 = p;

	numLights = 0;

	// point light
	lights[numLights].type = 0;
	lights[numLights].pos = { 4.0f, 6.0f, -2.0f, 1.0f };
	lights[numLights].dir = { 0.0f, 0.0f, 0.0f, 0.0f };
	lights[numLights].intensity = { 0.75f, 0.75f, 0.72f };
	numLights++;

	// directional light
	lights[numLights].type = 1;
	lights[numLights].pos = { 0.0f, 0.0f, 0.0f, 1.0f };
	lights[numLights].dir = { -0.4f, -1.0f, -0.35f, 0.0f };
	lights[numLights].intensity = { 0.85f, 0.85f, 0.8f };
	numLights++;

	for (int i = 0; i < MAX_SPHERES; i++) { vx[i] = vy[i] = vz[i] = 0.0f; }
	if (tracer.numSpheres > 0) { vx[0] = 0.6f;  vy[0] = 2.8f; vz[0] = -0.4f; }
	if (tracer.numSpheres > 1) { vx[1] = -0.7f; vy[1] = 3.4f; vz[1] = 0.3f; }
	if (tracer.numSpheres > 2) { vx[2] = 0.4f;  vy[2] = 2.2f; vz[2] = 0.5f; }
}

void raytrace::initCamera(int w, int h)
{
	viewpoint = makePoint(0.0f, 0.0f, 0.0f, 1.0f);
	pnear = -1.0f;
	fovx = (GLfloat)(M_PI / 3.0);  
	width = w;
	height = h;
}

void raytrace::drawScene(void)
{
	GLfloat imageWidth;
	point worldPix;
	point direction;
	ray r;
	color c;

	worldPix.w = 1.0;
	worldPix.z = pnear;
	r.start = &worldPix;
	r.end = &direction;

	imageWidth = 2.0f * fabsf(pnear) * tanf((float)(fovx / 2));

	for (int i = 0; i < width; i++) {
		for (int j = 0; j < height; j++) {
			worldPix.y = (j - (height / 2)) * imageWidth / width;
			worldPix.x = (i - (width / 2)) * imageWidth / width;
			calculateDirection(viewpoint, &worldPix, &direction);
			rayColor(&r, &c, 0);
			lowlevel.drawPixel(i, j, c.r, c.g, c.b);
		}
	}
}

void raytrace::stepPhysics() {
	const float dt = 0.02f;
	const int   substeps = 2;
	const float h = dt / substeps;
	const float g = -5.0f;
	const float floorRest = 0.85f;
	const float relaunch = 1.5f;

	// bounds that keep the balls in view
	const float xMin = -1.6f, xMax = 1.6f;
	const float zMin = -8.0f, zMax = -5.0f;

	for (int s = 0; s < substeps; s++) {
		for (int i = 0; i < tracer.numSpheres; i++) {
			sphere* sp = tracer.spheres[i];
			float r = sp->r;

			vy[i] += g * h;
			sp->c->x += vx[i] * h;
			sp->c->y += vy[i] * h;
			sp->c->z += vz[i] * h;

			// floor bounce
			float floorY = groundY + r;
			if (sp->c->y < floorY) {
				sp->c->y = floorY;
				vy[i] = -vy[i] * floorRest;
				if (vy[i] < relaunch) {
					vy[i] = 2.4f + 0.12f * (rand() % 9);
				}
			}

			// walls
			if (sp->c->x - r < xMin) { sp->c->x = xMin + r; vx[i] = fabsf(vx[i]); }
			if (sp->c->x + r > xMax) { sp->c->x = xMax - r; vx[i] = -fabsf(vx[i]); }
			if (sp->c->z - r < zMin) { sp->c->z = zMin + r; vz[i] = fabsf(vz[i]); }
			if (sp->c->z + r > zMax) { sp->c->z = zMax - r; vz[i] = -fabsf(vz[i]); }
		}

		// sphere-sphere collisions (equal mass, elastic)
		for (int i = 0; i < tracer.numSpheres; i++) {
			for (int j = i + 1; j < tracer.numSpheres; j++) {
				sphere* a = tracer.spheres[i];
				sphere* b = tracer.spheres[j];
				float dx = b->c->x - a->c->x;
				float dy = b->c->y - a->c->y;
				float dz = b->c->z - a->c->z;
				float dist = sqrtf(dx * dx + dy * dy + dz * dz);
				float minDist = a->r + b->r;
				if (dist < minDist && dist > 1e-5f) {
					float nx = dx / dist, ny = dy / dist, nz = dz / dist;

					// separate the overlap
					float overlap = 0.5f * (minDist - dist);
					a->c->x -= nx * overlap; a->c->y -= ny * overlap; a->c->z -= nz * overlap;
					b->c->x += nx * overlap; b->c->y += ny * overlap; b->c->z += nz * overlap;

					// swap velocity along the contact normal
					float vn = (vx[j] - vx[i]) * nx + (vy[j] - vy[i]) * ny + (vz[j] - vz[i]) * nz;
					if (vn < 0.0f) {
						float imp = vn;
						vx[i] += imp * nx; vy[i] += imp * ny; vz[i] += imp * nz;
						vx[j] -= imp * nx; vy[j] -= imp * ny; vz[j] -= imp * nz;
					}
				}
			}
		}
	}
}

point* raytrace::makePoint(GLfloat x, GLfloat y, GLfloat z, GLfloat w) {
	point* p = new point();
	p->x = x; p->y = y; p->z = z; p->w = w;
	return p;
}

sphere* raytrace::makeSphere(GLfloat x, GLfloat y, GLfloat z, GLfloat r) {
	sphere* s = new sphere();
	s->c = makePoint(x, y, z, 1.0f);
	s->r = r;
	s->m = NULL;
	return s;
}

void raytrace::calculateDirection(point* p, point* q, point* v) {
	v->x = q->x - p->x;
	v->y = q->y - p->y;
	v->z = q->z - p->z;
	v->w = 0.0;
}

bool raytrace::shadowed(point* p, vector* n, lightSource* light) {
	point start;
	start.x = p->x + SHADOW_EPSILON * n->x;
	start.y = p->y + SHADOW_EPSILON * n->y;
	start.z = p->z + SHADOW_EPSILON * n->z;
	start.w = 1.0f;

	vector dir;
	float lightDist;
	if (light->type == 1) {
		dir.x = -light->dir.x; dir.y = -light->dir.y; dir.z = -light->dir.z;
		lightDist = 1e30f;   // directional
	}
	else {
		dir.x = light->pos.x - p->x;
		dir.y = light->pos.y - p->y;
		dir.z = light->pos.z - p->z;
		lightDist = sqrtf(dir.x * dir.x + dir.y * dir.y + dir.z * dir.z);
	}
	dir.w = 0.0f;

	ray sray;
	sray.start = &start;
	sray.end = &dir;

	point hit; vector hn; material* hm;
	hit.w = 0.0f;
	tracer.trace(&sray, &hit, &hn, &hm);

	if (hit.w == 0.0f) return false;

	float dx = hit.x - p->x, dy = hit.y - p->y, dz = hit.z - p->z;
	float hitDist = sqrtf(dx * dx + dy * dy + dz * dz);
	return (hitDist < lightDist);
}

void raytrace::rayColor(ray* r, color* c, int depth)
{
	point p;
	vector n;
	material* m;

	p.w = 0.0f;
	tracer.trace(r, &p, &n, &m);

	if (p.w == 0.0f) {
		// background
		float dirLen = sqrtf(r->end->x * r->end->x + r->end->y * r->end->y + r->end->z * r->end->z);
		float t = 0.5f * (r->end->y / dirLen + 1.0f);
		c->r = (1.0f - t) * 1.0f + t * 0.35f;
		c->g = (1.0f - t) * 1.0f + t * 0.6f;
		c->b = (1.0f - t) * 1.0f + t * 1.0f;
		return;
	}

	float dirLen = sqrtf(r->end->x * r->end->x + r->end->y * r->end->y + r->end->z * r->end->z);
	vector normDir = { r->end->x / dirLen, r->end->y / dirLen, r->end->z / dirLen, 0.0f };
	vector viewDir = { -normDir.x, -normDir.y, -normDir.z, 0.0f };

	shader.shadeAmbient(&p, m, c);
	for (int i = 0; i < numLights; i++) {
		bool inShadow = shadowed(&p, &n, &lights[i]);
		shader.addLight(&p, &n, m, &viewDir, &lights[i], inShadow, c);
	}

	if (depth >= MAX_DEPTH) {
		if (c->r > 1.0f) c->r = 1.0f;
		if (c->g > 1.0f) c->g = 1.0f;
		if (c->b > 1.0f) c->b = 1.0f;
		return;
	}

	// reflection
	if (m->refl > 0.0f) {
		float dDotN = normDir.x * n.x + normDir.y * n.y + normDir.z * n.z;
		vector reflDir = {
			normDir.x - 2.0f * dDotN * n.x,
			normDir.y - 2.0f * dDotN * n.y,
			normDir.z - 2.0f * dDotN * n.z, 0.0f };

		point reflStart = {
			p.x + SHADOW_EPSILON * n.x,
			p.y + SHADOW_EPSILON * n.y,
			p.z + SHADOW_EPSILON * n.z, 1.0f };

		ray reflRay; reflRay.start = &reflStart; reflRay.end = &reflDir;
		color reflColor;
		rayColor(&reflRay, &reflColor, depth + 1);

		if (m->metal) {
			reflColor.r *= m->c.r;
			reflColor.g *= m->c.g;
			reflColor.b *= m->c.b;
		}

		c->r = (1.0f - m->refl) * c->r + m->refl * reflColor.r;
		c->g = (1.0f - m->refl) * c->g + m->refl * reflColor.g;
		c->b = (1.0f - m->refl) * c->b + m->refl * reflColor.b;
	}

	// refraction (Snell's law)
	if (m->refr > 0.0f) {
		float nx = n.x, ny = n.y, nz = n.z;
		float n1 = 1.0f, n2 = m->ior;

		float cosI = -(normDir.x * nx + normDir.y * ny + normDir.z * nz);
		if (cosI < 0.0f) {
			nx = -nx; ny = -ny; nz = -nz;
			cosI = -cosI;
			n1 = m->ior; n2 = 1.0f;
		}

		float eta = n1 / n2;
		float k = 1.0f - eta * eta * (1.0f - cosI * cosI);

		if (k >= 0.0f) {   
			float sqrtK = sqrtf(k);
			vector refrDir = {
				eta * normDir.x + (eta * cosI - sqrtK) * nx,
				eta * normDir.y + (eta * cosI - sqrtK) * ny,
				eta * normDir.z + (eta * cosI - sqrtK) * nz, 0.0f };

			point refrStart = {
				p.x - SHADOW_EPSILON * nx,
				p.y - SHADOW_EPSILON * ny,
				p.z - SHADOW_EPSILON * nz, 1.0f };

			ray refrRay; refrRay.start = &refrStart; refrRay.end = &refrDir;
			color refrColor;
			rayColor(&refrRay, &refrColor, depth + 1);

			c->r = (1.0f - m->refr) * c->r + m->refr * refrColor.r;
			c->g = (1.0f - m->refr) * c->g + m->refr * refrColor.g;
			c->b = (1.0f - m->refr) * c->b + m->refr * refrColor.b;
		}
	}

	if (c->r > 1.0f) c->r = 1.0f;
	if (c->g > 1.0f) c->g = 1.0f;
	if (c->b > 1.0f) c->b = 1.0f;
}
