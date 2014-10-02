#ifndef PLAYER_HPP
#define PLAYER_HPP
#include "Entity.hpp"

class Camera;
class Player : public Entity {
	public:
		Player();
		~Player();

		void update(float deltaTime);
		Camera* getCam() const { return cam; }

	private:
		void processKeys();
		void traceView();

		Camera* cam;
		unsigned int selectedID; //current blockID, used to place blocks
		vec3i targetedBlock;
		vec3i targetedBlockEnter;
		float xRot;
		bool onFloor;
		bool isJumping;
		bool targetsBlock;
};

#endif // PLAYER_HPP
