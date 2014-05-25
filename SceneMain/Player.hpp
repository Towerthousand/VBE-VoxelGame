#ifndef PLAYER_HPP
#define PLAYER_HPP
#include "Entity.hpp"
#include "PlayerBase.hpp"

class Camera;
class Player : public PlayerBase {
	public:
		Player();
		~Player();

		virtual void update(float deltaTime);
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
