#include "Player.hpp"
#include "Camera.hpp"
#include "world/World.hpp"
#include "world/DeferredCubeLight.hpp"
#include "DeferredContainer.hpp"

Player::Player() : cam(nullptr), selectedID(0), targetedBlock(0.0f), targetedBlockEnter(0.0f), onFloor(true), isJumping(false), targetsBlock(false) {
	setName("Player");
	cam = new Camera("playerCam", vec3f(0,1.5,0));
	cam->addTo(this);
	acc = vec3f(0,-10,0);
	pos = vec3f(8,110,8);
	hitbox.type = Hitbox::BOX;
	hitbox.radius = vec3f(0.6*scale.x,1.6*scale.y,0.6*scale.z);
}

Player::~Player() {
}

void Player::update(float deltaTime) {
	//take input
	processKeys();

	//move and update camera position
	movePos(deltaTime); //this handles collisions

	//feedback to be used by the scene
	onFloor = hitbox.collidesWithWorld(vec3f(0,-0.1,0));
	isJumping = (vel.y > 0);

	//Limit movement
	vel.x = 0; // Player only accelerates vertically, so speed.x doesn't carry
	vel.y = std::fmax(-70,vel.y);
	vel.z = 0; // Player only accelerates vertically, so speed.z doesn't carry

	//transform coordinates for camera and other children
	transform = glm::translate(mat4f(1.0),pos);

	//trace view
	traceView();
}

void Player::processKeys() {
	World* w = (World*)getGame()->getObjectByName("World");
	//Move player
	const float speed = 10.0f;
	vec2f dir(sin(cam->rot.y*DEG_TO_RAD), -cos(cam->rot.y*DEG_TO_RAD));
	if(Input::isKeyDown(sf::Keyboard::W)) {
		vel.x += dir.x*speed;
		vel.z += dir.y*speed;
	}
	if(Input::isKeyDown(sf::Keyboard::S)) {
		vel.x += -dir.x*speed;
		vel.z += -dir.y*speed;
	}
	if(Input::isKeyDown(sf::Keyboard::A)) {
		vel.x += dir.y*speed;
		vel.z += -dir.x*speed;
	}
	if(Input::isKeyDown(sf::Keyboard::D)) {
		vel.x += -dir.y*speed;
		vel.z += dir.x*speed;
	}
	if(Input::isKeyDown(sf::Keyboard::Space))
		if (onFloor && !isJumping)
			vel.y = 15;

	if(Input::isKeyDown(sf::Keyboard::C)) {
		cam->pos.y = 0.5;
	}
	else {
		cam->pos.y = 1.5;
	}
	//look around
	if(Input::getMouseDisplacement() != vec2i(0, 0))
		cam->rot += vec3f(Input::getMouseDisplacement().y*0.1f, Input::getMouseDisplacement().x*0.1f, 0);
	Input::setMousePos(SCRWIDTH/2, SCRHEIGHT/2, getGame()->getWindow());

	bool recalc = false;
	//take block
	if(Input::isMousePressed(sf::Mouse::Left))
		if(targetsBlock) {
			w->setCube(targetedBlock.x,targetedBlock.y,targetedBlock.z,0);
			recalc = true;
		}

	//put block
	if(Input::isMousePressed(sf::Mouse::Right))
		if(targetsBlock) {
			VBE_LOG(targetedBlockEnter.x << " " << targetedBlockEnter.y << " " << targetedBlockEnter.z);
			w->setCube(targetedBlockEnter.x,targetedBlockEnter.y,targetedBlockEnter.z,1);
			recalc = true;
		}

	if(recalc)
	{
		std::vector<DeferredCubeLight*> lights;
		getGame()->getAllObjectsOfType(lights);
		for(int i = 0; i < lights.size(); i++)
			lights[i]->calcLight();
	}

	if(Input::isKeyPressed(sf::Keyboard::L)) {
		Camera* cam = (Camera*)getGame()->getObjectByName("playerCam");
		DeferredContainer* renderer = (DeferredContainer*)getGame()->getObjectByName("deferred");
		vec3f pos = glm::floor(cam->getWorldPos())+vec3f(0.5f);
		DeferredCubeLight* l = new DeferredCubeLight(pos, glm::abs(glm::sphericalRand(1.0f)));
		l->addTo(renderer);
	}
}

void Player::traceView() {
	World* w = (World*)getGame()->getObjectByName("World");
    float tMax = 10; //View radius
	vec3f   cpos(cam->getWorldPos()),
			dir(cos(-cam->rot.x*DEG_TO_RAD)*(-sin(-cam->rot.y*DEG_TO_RAD)),
				sin(-cam->rot.x*DEG_TO_RAD),
				-cos(-cam->rot.x*DEG_TO_RAD)*cos(-cam->rot.y*DEG_TO_RAD)),
			vox(floor(cpos.x), floor(cpos.y), floor(cpos.z)),
			step(0,0,0),
			next(0,0,0),
			tMaxc(tMax,tMax,tMax),
			tDelta(tMax,tMax,tMax);

	if (!w->outOfBounds(cpos.x,cpos.y,cpos.z) &&
		w->getCube(cpos.x,cpos.y,cpos.z) != 0) {
		targetsBlock = true;
		targetedBlock = vec3f(floor(cpos.x),floor(cpos.y),floor(cpos.z));
		return;
	}

	if (dir.x < 0) step.x = -1;
	else step.x = 1;
	if (dir.y < 0) step.y = -1;
	else step.y = 1;
	if (dir.z < 0) step.z = -1;
	else step.z = 1;

	next.x = vox.x + (step.x > 0 ? 1 : 0);
	next.y = vox.y + (step.y > 0 ? 1 : 0);
	next.z = vox.z + (step.z > 0 ? 1 : 0);

	if (dir.x != 0) {
		tDelta.x = step.x/dir.x;
		tMaxc.x = (next.x - cpos.x)/dir.x;
	}
	if (dir.y != 0) {
		tDelta.y = step.y/dir.y;
		tMaxc.y = (next.y - cpos.y)/dir.y;
	}
	if (dir.z != 0) {
		tDelta.z = step.z/dir.z;
		tMaxc.z = (next.z - cpos.z)/dir.z;
	}

	float tCurr = 0;
	while (tCurr < tMax) {
		targetedBlockEnter = vox;
		if(tMaxc.x < tMaxc.y) {
			if(tMaxc.x < tMaxc.z) {
				tCurr = tMaxc.x;
				tMaxc.x = tMaxc.x + tDelta.x;
				vox.x = vox.x + step.x;
			}
			else {
				tCurr = tMaxc.z;
				vox.z = vox.z + step.z;
				tMaxc.z = tMaxc.z + tDelta.z;
			}
		}
		else {
			if(tMaxc.y < tMaxc.z) {
				tCurr = tMaxc.y;
				vox.y = vox.y + step.y;
				tMaxc.y = tMaxc.y + tDelta.y;
			}
			else {
				tCurr = tMaxc.z;
				vox.z = vox.z + step.z;
				tMaxc.z= tMaxc.z + tDelta.z;
			}
		}
		if(!w->outOfBounds(vox.x,vox.y,vox.z) && w->getCube(vox.x,vox.y,vox.z) != 0) {
			targetsBlock = true;
			targetedBlock = vox;
			return;
		}
	}
	targetsBlock = false;
}
