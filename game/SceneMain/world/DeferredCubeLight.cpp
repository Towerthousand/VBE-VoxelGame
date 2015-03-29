#include "DeferredCubeLight.hpp"
#include "World.hpp"
#include "../DeferredContainer.hpp"
#include "../Manager.hpp"

DeferredCubeLight::DeferredCubeLight(const vec3f& pos, const vec3f& color) : pos(pos), color(color), renderer(nullptr), world(nullptr) {
	renderer = (DeferredContainer*)getGame()->getObjectByName("deferred");
	world = (World*)getGame()->getObjectByName("world");
	tex = Texture3D(vec3ui(LIGHTSIZE*2), TextureFormat::RED);

	int x0 = int(floor(pos.x));
	int y0 = int(floor(pos.y));
	int z0 = int(floor(pos.z));
	calcLight(x0, y0, z0);

	quad = &Meshes.get("quad");
}

DeferredCubeLight::~DeferredCubeLight() {
}

void DeferredCubeLight::count(std::pair<float, float>& pr, vec3i p, vec3i c) {
	float c1 = glm::dot(p, c);
	float c2 = glm::dot(p, p);
	vec3f b = vec3f(p) * (c1 / c2);

	float d = glm::distance(b, vec3f(c));

	float w = 1-d;
	if(w < 0) w = 0;

	if(w == 0) return;

	pr.first += light[c.x+LIGHTSIZE][c.y+LIGHTSIZE][c.z+LIGHTSIZE]*w;
	pr.second += w;
}

inline int sign(int n) {
	return n<0?-1:1;
}

void DeferredCubeLight::calcQuadrant(int cx, int cy, int cz, int dx, int dy, int dz) {
	if(cx*dx < 0 || cy*dy < 0 || cz*dz < 0)
		return;

	int x0 = int(floor(pos.x));
	int y0 = int(floor(pos.y));
	int z0 = int(floor(pos.z));

	for(int x = cx; x >= -LIGHTSIZE && x < LIGHTSIZE; x+=dx) {
		for(int y = cy; y >= -LIGHTSIZE && y < LIGHTSIZE; y+=dy) {
			for(int z = cz; z >= -LIGHTSIZE && z < LIGHTSIZE; z+=dz) {

				if((x == 0 && y == 0 && z == 0) || sqrt(x*x + y*y + z*z) > LIGHTSIZE) continue;

				if(world->getCube(x+x0, y+y0, z+z0) != 0)
					light[x+LIGHTSIZE][y+LIGHTSIZE][z+LIGHTSIZE] = 0;
				else
				{
					std::pair<float, float> p (0, 0);
					count(p, vec3i(x, y, z), vec3i(x-dx, y, z));
					count(p, vec3i(x, y, z), vec3i(x, y-dy, z));
					count(p, vec3i(x, y, z), vec3i(x, y, z-dz));
					count(p, vec3i(x, y, z), vec3i(x-dx, y-dy, z));
					count(p, vec3i(x, y, z), vec3i(x-dx, y, z-dz));
					count(p, vec3i(x, y, z), vec3i(x, y-dy, z-dz));
					count(p, vec3i(x, y, z), vec3i(x-dx, y-dy, z-dz));

					p.first /= p.second;

					light[x+LIGHTSIZE][y+LIGHTSIZE][z+LIGHTSIZE] = p.first;
				}
				data[z+LIGHTSIZE][y+LIGHTSIZE][x+LIGHTSIZE] = (unsigned char)(light[x+LIGHTSIZE][y+LIGHTSIZE][z+LIGHTSIZE]*255);
			}
		}
	}
}

void DeferredCubeLight::calcLight(int cx, int cy, int cz) {

	light[LIGHTSIZE][LIGHTSIZE][LIGHTSIZE] = 1;
	data[LIGHTSIZE][LIGHTSIZE][LIGHTSIZE] = (unsigned char)(light[LIGHTSIZE][LIGHTSIZE][LIGHTSIZE]*255);

	int x0 = int(floor(pos.x));
	int y0 = int(floor(pos.y));
	int z0 = int(floor(pos.z));

	cx -= x0;
	cy -= y0;
	cz -= z0;

	calcQuadrant(cx, cy, cz, -1, -1, -1);
	calcQuadrant(cx, cy, cz, -1, -1,  1);
	calcQuadrant(cx, cy, cz, -1,  1, -1);
	calcQuadrant(cx, cy, cz, -1,  1,  1);
	calcQuadrant(cx, cy, cz,  1, -1, -1);
	calcQuadrant(cx, cy, cz,  1, -1,  1);
	calcQuadrant(cx, cy, cz,  1,  1, -1);
	calcQuadrant(cx, cy, cz,  1,  1,  1);
	tex.setData(data, TextureFormat::RED, TextureFormat::UNSIGNED_BYTE);
	tex.setFilter(GL_LINEAR,GL_LINEAR);
	tex.setWrap(GL_CLAMP_TO_BORDER);
}

void DeferredCubeLight::update(float deltaTime) {
	(void) deltaTime;
	transform = glm::translate(mat4f(1.0f), pos);
}

void DeferredCubeLight::draw() const {
	if(renderer->getMode() != DeferredContainer::Light) return;
	Camera* cam = (Camera*)getGame()->getObjectByName("playerCam");
	vec3f posWorldSpace = vec3f(fullTransform*vec4f(0,0,0,1));
	vec3f posViewSpace = vec3f(cam->getView()*vec4f(posWorldSpace,1.0));

	mat4f t(1.0);
	if(glm::length(posViewSpace) > LIGHTSIZE) {
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
		t = glm::scale(rot, vec3f(LIGHTSIZE));
		t = glm::translate(t, vec3f(0, 0, 1));
		Programs.get("deferredCubeLight").uniform("MVP")->set(cam->projection*cam->getView()*fullTransform*t);
	}
	else
		Programs.get("deferredCubeLight").uniform("MVP")->set(t);

	Programs.get("deferredCubeLight").uniform("invResolution")->set(vec2f(1.0f/Window::getInstance()->getSize().x, 1.0f/Window::getInstance()->getSize().y));
	Programs.get("deferredCubeLight").uniform("color0")->set(renderer->getColor0());
	Programs.get("deferredCubeLight").uniform("color1")->set(renderer->getColor1());
	Programs.get("deferredCubeLight").uniform("depth")->set(renderer->getDepth());
	Programs.get("deferredCubeLight").uniform("lightPos")->set(posViewSpace);
	Programs.get("deferredCubeLight").uniform("invProj")->set(glm::inverse(cam->projection));
	Programs.get("deferredCubeLight").uniform("invView")->set(glm::inverse(cam->getView()));
	Programs.get("deferredCubeLight").uniform("lightColor")->set(color);
	Programs.get("deferredCubeLight").uniform("lightRadius")->set(float(LIGHTSIZE));
	Programs.get("deferredCubeLight").uniform("tex")->set(tex);
	quad->draw(Programs.get("deferredCubeLight"));
}
