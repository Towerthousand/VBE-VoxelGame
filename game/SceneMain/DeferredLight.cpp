#include "DeferredLight.hpp"
#include "DeferredContainer.hpp"
#include "Manager.hpp"

DeferredLight::DeferredLight() : pos(0.0f), color(1.0f), radius(30.0f), renderer((DeferredContainer*)getGame()->getObjectByName("deferred")) {
	quad = Meshes.get("quad");
}

DeferredLight::~DeferredLight() {
}

void DeferredLight::update(float deltaTime) {
	(void) deltaTime;
}

void DeferredLight::draw() const{
	if(renderer->getMode() != DeferredContainer::Light) return;
	Camera* cam = (Camera*)getGame()->getObjectByName("playerCam");
	vec3f posWorldSpace = vec3f(fullTransform*vec4f(0,0,0,1));
	vec3f posViewSpace = vec3f(cam->getView()*vec4f(posWorldSpace,1.0));

	mat4f t(1.0);
	if(glm::length(posViewSpace) > radius) {
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
		t = glm::scale(rot, vec3f(radius));
		t = glm::translate(t, vec3f(0, 0, 1));
		Programs.get("deferredLight")->uniform("MVP")->set(cam->projection*cam->getView()*fullTransform*t);
	}
	else
		Programs.get("deferredLight")->uniform("MVP")->set(t);

	Programs.get("deferredLight")->uniform("invResolution")->set(vec2f(1.0f/Window::getInstance()->getSize().x, 1.0f/Window::getInstance()->getSize().y));
	Programs.get("deferredLight")->uniform("color0")->set(renderer->getColor0());
	Programs.get("deferredLight")->uniform("color1")->set(renderer->getColor1());
	Programs.get("deferredLight")->uniform("depth")->set(renderer->getDepth());
	Programs.get("deferredLight")->uniform("lightPos")->set(posViewSpace);
	Programs.get("deferredLight")->uniform("invProj")->set(glm::inverse(cam->projection));
	Programs.get("deferredLight")->uniform("lightColor")->set(color);
	Programs.get("deferredLight")->uniform("lightRadius")->set(radius);
	quad->draw(Programs.get("deferredLight"));
}
