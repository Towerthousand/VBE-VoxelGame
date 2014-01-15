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
		unsigned char selectedID; //current blockID, used to place blocks
		vec3i targetedBlock;
		vec3i targetedBlockEnter;
		bool onFloor;
		bool isJumping;
		bool targetsBlock;
};

#endif // PLAYER_HPP
