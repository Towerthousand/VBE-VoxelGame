#include "PlayerCamera.hpp"

PlayerCamera::PlayerCamera(const std::string& cameraName, const vec3f& pos, const vec3f& rot) :
	Camera(cameraName, pos, rot) {
	setName(cameraName);
}

void PlayerCamera::update(float deltaTime) {
	//move player camera
	vec3f dirFront(sin(rot.y*DEG_TO_RAD)*cos(rot.x*DEG_TO_RAD), -sin(rot.x*DEG_TO_RAD), -cos(rot.y*DEG_TO_RAD)*cos(rot.x*DEG_TO_RAD));
	vec3f dirRight(-cos(rot.y*DEG_TO_RAD), 0, -sin(rot.y*DEG_TO_RAD));
	float vel = 10.0f;
	if(Input::isKeyDown(sf::Keyboard::S)) pos -= dirFront*deltaTime*vel;
	if(Input::isKeyDown(sf::Keyboard::W)) pos += dirFront*deltaTime*vel;
	if(Input::isKeyDown(sf::Keyboard::D)) pos -= dirRight*deltaTime*vel;
	if(Input::isKeyDown(sf::Keyboard::A)) pos += dirRight*deltaTime*vel;
	if(Input::getMouseDisplacement() != vec2i(0, 0))
		rot += vec3f(Input::getMouseDisplacement().y*0.1f, Input::getMouseDisplacement().x*0.1f, 0);
	//center mouse
	Input::setMousePos(SCRWIDTH/2, SCRHEIGHT/2, getGame()->getWindow());
	Camera::update(deltaTime);
}
