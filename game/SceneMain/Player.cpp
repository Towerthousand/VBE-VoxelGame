#include "Player.hpp"
#include "world/World.hpp"
#include "world/Chunk.hpp"
#include "world/DeferredCubeLight.hpp"
#include "DeferredContainer.hpp"

Player::Player() : cam(nullptr), selectedID(0), targetedBlock(0.0f), targetedBlockEnter(0.0f), xRot(0.0f), onFloor(true), isJumping(false), targetsBlock(false) {
	setName("player");
	cam = new Camera("playerCam", vec3f(0,1.5,0));
	cam->addTo(this);
	acc = vec3f(0,-10,0);
	pos = vec3f(0,256,0);
	hitbox->type = Hitbox::BOX;
	hitbox->radius = vec3f(0.6*scale.x,1.6*scale.y,0.6*scale.z);
}

Player::~Player() {
}

void Player::update(float deltaTime) {
	//take input
	if(!Profiler::isShown()) processKeys();

	//move and update camera position
	if(!Profiler::isShown()) movePos(deltaTime); //this handles collisions

	//feedback to be used by the scene
	onFloor = hitbox->collidesWithWorld(vec3f(0,-0.1,0));
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
	World* w = (World*)getGame()->getObjectByName("world");
	//Move player
	const float speedKeys = 10.0f;
	vec2f speedPad = vec2f(Gamepad::axis(0, Gamepad::AxisLeftX), Gamepad::axis(0, Gamepad::AxisLeftY));
	vec2f dir = vec2f(cam->getForward().x,cam->getForward().z);
	dir = (dir == vec2f(0.0f))? vec2f(1.0f,0.0f) : glm::normalize(dir);
	if(Keyboard::pressed(Keyboard::W)) {
		vel.x += dir.x*speedKeys;
		vel.z += dir.y*speedKeys;
	}
	if(Keyboard::pressed(Keyboard::S)) {
		vel.x += -dir.x*speedKeys;
		vel.z += -dir.y*speedKeys;
	}
	if(Keyboard::pressed(Keyboard::A)) {
		vel.x += dir.y*speedKeys;
		vel.z += -dir.x*speedKeys;
	}
	if(Keyboard::pressed(Keyboard::D)) {
		vel.x += -dir.y*speedKeys;
		vel.z += dir.x*speedKeys;
	}
	if(glm::length(speedPad) > 0.3f) {
		vel.x += -dir.x*speedPad.y*speedKeys;
		vel.z += -dir.y*speedPad.y*speedKeys;
		vel.x += -dir.y*speedPad.x*speedKeys;
		vel.z += dir.x*speedPad.x*speedKeys;
	}
	if(Keyboard::pressed(Keyboard::Space) || Gamepad::pressed(0, Gamepad::ButtonA))
		//if (onFloor && !isJumping)
		vel.y = 15;

	//look around
	vec2f displacement = vec2f(Mouse::movement())*0.1f;
	vec2f padDisplacement = vec2f(Gamepad::axis(0, Gamepad::AxisRightX), Gamepad::axis(0, Gamepad::AxisRightY));
	if(glm::length(padDisplacement) > 0.3f) //death zone
		displacement += padDisplacement * 2.0f;

	cam->rotateGlobal(displacement.x, vec3f(0,1,0));
	//limit x rotation
	if(std::abs(xRot+displacement.y) < 90.0f) {
		cam->rotateLocal(displacement.y, vec3f(1,0,0));
		xRot += displacement.y;
	}
	//TODO Sacar la logica de recalcular luces aqui, tendria que hacerse en setCube
	bool recalc = false;
	vec3i recalcBlock;

	//take block
	if(Mouse::justPressed(Mouse::Left)) {
		if(targetsBlock) {
			for(int i = -10; i <= 10; ++i)
				for(int j = -10; j <= 10; ++j)
					for(int k = -10; k <= 10; ++k)
						w->setCube(targetedBlock.x+i, targetedBlock.y+j, targetedBlock.z+k, 0);
			recalcBlock = targetedBlock;
			recalc = true;
		}
	}

	//put block
	if(Mouse::justPressed(Mouse::Right))
		if(targetsBlock) {
			w->setCube(targetedBlockEnter.x,targetedBlockEnter.y,targetedBlockEnter.z,1);
			recalcBlock = targetedBlockEnter;
			recalc = true;
		}

	if(recalc) {
		std::vector<DeferredCubeLight*> lights;
		getGame()->getAllObjectsOfType(lights);
		for(unsigned int i = 0; i < lights.size(); i++) {
			DeferredCubeLight* l = lights[i];
			l->calcLight(l->getPosition().x, l->getPosition().y, l->getPosition().z);
		}
	}

	if(Keyboard::justPressed(Keyboard::L)) {
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
