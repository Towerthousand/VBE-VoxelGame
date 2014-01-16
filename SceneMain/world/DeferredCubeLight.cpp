#include "DeferredCubeLight.hpp"
#include "World.hpp"
#include "../Camera.hpp"
#include "../DeferredContainer.hpp"

#define SIZE 16

DeferredCubeLight::DeferredCubeLight(const vec3f& pos, const vec3f& color) : pos(pos), color(color), renderer(nullptr), world(nullptr) {
	renderer = (DeferredContainer*)getGame()->getObjectByName("deferred");
	world = (World*)getGame()->getObjectByName("World");

	calcLight();

	quad.mesh = Meshes.get("quad");
	quad.program = Programs.get("deferredCubeLight");
}

DeferredCubeLight::~DeferredCubeLight() {
}


void DeferredCubeLight::count(float light[SIZE*2][SIZE*2][SIZE*2], std::pair<double, double>& p, double px, double py, double pz, int x, int y, int z)
{
//	if(x < 0 || y < 0 || z < 0)
//		return;

	double c1 = x*px+y*py+z*pz;
	double c2 = px*px+py*py+pz*pz;
	double b = c1 / c2;

	double bx = px*b;
	double by = py*b;
	double bz = pz*b;

	double d = sqrt((x-bx)*(x-bx)+(y-by)*(y-by)+(z-bz)*(z-bz));

	double w = 1-d;
	if(w < 0) w = 0;

	if(w == 0) return;

	p.first += light[x+SIZE][y+SIZE][z+SIZE]*w;
	p.second += w;
}

int sign(int n)
{
	return n<0?-1:1;
}


void DeferredCubeLight::calcLight() {
	float light[SIZE*2][SIZE*2][SIZE*2];

	int x0 = int(floor(pos.x));
	int y0 = int(floor(pos.y));
	int z0 = int(floor(pos.z));

	light[SIZE][SIZE][SIZE] = 1;

	for(int x2 = -SIZE; x2 < SIZE; x2++)
	{
		int x = x2 <= 0 ? -x2 - SIZE : x2;
		int dx = sign(x);
		for(int y2 = -SIZE; y2 < SIZE; y2++)
		{
			int y = y2 <= 0 ? -y2 - SIZE : y2;
			int dy = sign(y);
			for(int z2 = -SIZE; z2 < SIZE; z2++)
			{
				int z = z2 <= 0 ? -z2 - SIZE : z2;
				int dz = sign(z);

				if(x == 0 && y == 0 && z == 0) continue;

				if(world->getCube(x+x0, y+y0, z+z0).ID != 0)
				{
					light[x+SIZE][y+SIZE][z+SIZE] = 0;
					continue;
				}

				std::pair<double, double> p (0, 0);
				count(light, p, x, y, z, x-dx, y, z);
				count(light, p, x, y, z, x, y-dy, z);
				count(light, p, x, y, z, x, y, z-dz);
				count(light, p, x, y, z, x-dx, y-dy, z);
				count(light, p, x, y, z, x-dx, y, z-dz);
				count(light, p, x, y, z, x, y-dy, z-dz);
				count(light, p, x, y, z, x-dx, y-dy, z-dz);

				p.first /= p.second;

				light[x+SIZE][y+SIZE][z+SIZE] = p.first;
			}
		}
	}

	unsigned char data[SIZE*2][SIZE*2][SIZE*2];

	for(int x = 0; x < SIZE*2; x++)
		for(int y = 0; y < SIZE*2; y++)
			for(int z = 0; z < SIZE*2; z++)
				data[z][y][x] = (unsigned char)(light[x][y][z]*255);

	tex.loadFromRaw(data, SIZE*2, SIZE*2, SIZE*2, Texture::RED, Texture::UNSIGNED_BYTE, Texture::R8);
	tex.setFilter(GL_NEAREST,GL_NEAREST);
}

void DeferredCubeLight::update(float deltaTime) {
	(void) deltaTime;
	transform = glm::translate(mat4f(1.0f), pos);
}

void DeferredCubeLight::draw() const {
	if(renderer->getMode() != DeferredContainer::Light) return;
	Camera* cam = (Camera*)getGame()->getObjectByName("playerCam");
	vec3f posWorldSpace = vec3f(fullTransform*vec4f(0,0,0,1));
	vec3f posViewSpace = vec3f(cam->view*vec4f(posWorldSpace,1.0));

	mat4f t(1.0);
	if(glm::length(posViewSpace) > SIZE) {
		vec3f front = cam->getWorldPos()-posWorldSpace;
		front = glm::normalize(front);
		vec3f dummyUp(0, 1, 0);
		vec3f right = glm::cross(dummyUp, front);
		right = glm::normalize(right);
		vec3f up = glm::cross(front, right);
		up = glm::normalize(up);
		mat4f rot(right.x, right.y, right.z, 0,
				  up.x   , up.y   , up.z   , 0,
				  front.x, front.y, front.z, 0,
				  0      , 0      , 0      , 1);
		t = glm::scale(rot, vec3f(SIZE));
		t = glm::translate(t, vec3f(0, 0, 1));
		quad.program->uniform("MVP")->set(cam->projection*cam->view*fullTransform*t);
	}
	else
		quad.program->uniform("MVP")->set(t);

	quad.program->uniform("invResolution")->set(vec2f(1.0f/SCRWIDTH, 1.0f/SCRHEIGHT));
	quad.program->uniform("color0")->set(renderer->getColor0());
	quad.program->uniform("color1")->set(renderer->getColor1());
	quad.program->uniform("depth")->set(renderer->getDepth());
	quad.program->uniform("lightPos")->set(posViewSpace);
	quad.program->uniform("invProj")->set(glm::inverse(cam->projection));
	quad.program->uniform("invView")->set(glm::inverse(cam->view));
	quad.program->uniform("lightColor")->set(color);
	quad.program->uniform("lightRadius")->set(float(SIZE));
	quad.program->uniform("tex")->set(&tex);
	quad.draw();
}
