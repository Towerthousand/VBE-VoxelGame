#include "DeferredCubeLight.hpp"
#include "World.hpp"
#include "../Camera.hpp"
#include "../DeferredContainer.hpp"

DeferredCubeLight::DeferredCubeLight(const vec3f& pos, const vec3f& color) : pos(pos), color(color), renderer(nullptr), world(nullptr) {
	renderer = (DeferredContainer*)getGame()->getObjectByName("deferred");
	world = (World*)getGame()->getObjectByName("World");

	calcLight();
	tex.setFilter(GL_NEAREST,GL_NEAREST);

	quad.mesh = Meshes.get("quad");
	quad.program = Programs.get("deferredCubeLight");
}

DeferredCubeLight::~DeferredCubeLight() {
}

void DeferredCubeLight::calcLight() {
	unsigned char data[32][32][32];

	int x0 = int(floor(pos.x));
	int y0 = int(floor(pos.y));
	int z0 = int(floor(pos.z));

	for(int x = -16; x < 16; x++)
		for(int y = -16; y < 16; y++)
			for(int z = -16; z < 16; z++)
			{
				if(world->getCube(x+x0, y+y0, z+z0).ID != 0)
					data[z+16][y+16][x+16] = 0;
				else
					data[z+16][y+16][x+16] = 255;
			}

	//memset(data, 255, sizeof(data));
	tex.loadFromRaw(data, 32, 32, 32, Texture::RED, Texture::UNSIGNED_BYTE, Texture::R8);
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
	if(glm::length(posViewSpace) > 16.0f) {
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
		t = glm::scale(rot, vec3f(16.0f));
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
	quad.program->uniform("lightRadius")->set(16.0f);
	quad.program->uniform("tex")->set(&tex);
	quad.draw();
}
