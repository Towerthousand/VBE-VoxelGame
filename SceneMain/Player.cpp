#include "Player.hpp"
#include "world/World.hpp"
#include "world/DeferredCubeLight.hpp"
#include "DeferredContainer.hpp"
#include "NetworkManager.hpp"

Player::Player() : PlayerBase(-1), cam(nullptr), selectedID(0), targetedBlock(0.0f), targetedBlockEnter(0.0f), onFloor(true), isJumping(false), targetsBlock(false) {
	setName("player");
	cam = new Camera("playerCam", vec3f(0,1.0,0));
	cam->addTo(this);
	cam->projection = glm::perspective(60.0f, float(Environment::getScreen()->getWidth())/float(Environment::getScreen()->getHeight()), 0.01f, 10000.0f);
	acc = vec3f(0,-27,0);
	pos = vec3f(8,110,8);
	hitbox.type = Hitbox::BOX;
	hitbox.radius = vec3f(0.3*scale.x,0.8*scale.y,0.3*scale.z);
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
	//vel.y = std::fmax(-70,vel.y);
	vel.z = 0; // Player only accelerates vertically, so speed.z doesn't carry

	//transform coordinates for camera and other children
	transform = glm::translate(mat4f(1.0),pos);

	//trace view
	traceView();
}

void Player::processKeys() {
	World* w = (World*)getGame()->getObjectByName("world");
	NetworkManager* net = (NetworkManager*)getGame()->getObjectByName("NetworkManager");
	//Move player
	const float speed = 7.0f;
	vec2f dir = vec2f(cam->getForward().x,cam->getForward().z);
	dir = (dir == vec2f(0.0f))? vec2f(1.0f,0.0f) : glm::normalize(dir);
	if(Environment::getKeyboard()->isKeyHeld(Keyboard::W)) {
		vel.x += dir.x*speed;
		vel.z += dir.y*speed;
	}
	if(Environment::getKeyboard()->isKeyHeld(Keyboard::S)) {
		vel.x += -dir.x*speed;
		vel.z += -dir.y*speed;
	}
	if(Environment::getKeyboard()->isKeyHeld(Keyboard::A)) {
		vel.x += dir.y*speed;
		vel.z += -dir.x*speed;
	}
	if(Environment::getKeyboard()->isKeyHeld(Keyboard::D)) {
		vel.x += -dir.y*speed;
		vel.z += dir.x*speed;
	}
	if(Environment::getKeyboard()->isKeyHeld(Keyboard::Space))
		if (onFloor && !isJumping)
			vel.y = 8.4;

	//look around
	vec2f displacement = vec2f(Environment::getMouse()->getMousePosRelative())*0.2f;
	pitch += displacement.y;
	yaw += displacement.x;
	if(pitch < -90) pitch = -90;
	if(pitch > 90) pitch = 90;
	if(yaw < 0) yaw += 360;
	if(yaw >= 360) yaw -= 360;

	mat4f rot (1.0f);
	rot = glm::rotate(rot, pitch, vec3f(1, 0, 0));
	rot = glm::rotate(rot, yaw, vec3f(0, 1, 0));
	cam->rotation = rot;

	net->sendPosition(pos.x, pos.y, pos.z, yaw, pitch);
	//TODO Sacar la logica de recalcular luces aqui, tendria que hacerse en setBlock
	bool recalc = false;
	vec3i recalcBlock;

	heldBlock = 1; //TODO
	//take block
	if(Environment::getMouse()->isButtonPressed(Mouse::Left))
		if(targetsBlock) {
			w->setBlock(targetedBlock.x,targetedBlock.y,targetedBlock.z,0);
			net->sendSetBlock(targetedBlock.x,targetedBlock.y,targetedBlock.z,false, heldBlock);

			recalcBlock = targetedBlock;
			recalc = true;
		}

	//put block
	if(Environment::getMouse()->isButtonPressed(Mouse::Right))
		if(targetsBlock) {
			w->setBlock(targetedBlockEnter.x, targetedBlockEnter.y, targetedBlockEnter.z, heldBlock);
			net->sendSetBlock(targetedBlockEnter.x, targetedBlockEnter.y, targetedBlockEnter.z, true, heldBlock);
			recalcBlock = targetedBlockEnter;
			recalc = true;
		}

	if(recalc) {
		std::vector<DeferredCubeLight*> lights;
		getGame()->getAllObjectsOfType(lights);
		for(unsigned int i = 0; i < lights.size(); i++) {
			DeferredCubeLight* l = lights[i];
			l->calcLight(recalcBlock.x, recalcBlock.y, recalcBlock.z);
		}
	}

	if(Environment::getKeyboard()->isKeyHeld(Keyboard::L)) {
		Camera* cam = (Camera*)getGame()->getObjectByName("playerCam");
		DeferredContainer* renderer = (DeferredContainer*)getGame()->getObjectByName("deferred");
		vec3f pos = glm::floor(cam->getWorldPos())+vec3f(0.5f);
		DeferredCubeLight* l = new DeferredCubeLight(pos, glm::abs(glm::sphericalRand(1.0f)));
		l->addTo(renderer);
	}
}

void Player::traceView() {
	World* w = (World*)getGame()->getObjectByName("world");
	float tMax = 10; //View radius
	vec3f   cpos(cam->getWorldPos()),
			dir(cam->getForward()),
			vox(floor(cpos.x), floor(cpos.y), floor(cpos.z)),
			step(0,0,0),
			next(0,0,0),
			tMaxc(tMax,tMax,tMax),
			tDelta(tMax,tMax,tMax);

	if (!w->outOfBounds(cpos.x,cpos.y,cpos.z) &&
			w->getBlock(cpos.x,cpos.y,cpos.z) != 0) {
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
		if(!w->outOfBounds(vox.x,vox.y,vox.z) && w->getBlock(vox.x,vox.y,vox.z) != 0) {
			targetsBlock = true;
			targetedBlock = vox;
			return;
		}
	}
	targetsBlock = false;
}
