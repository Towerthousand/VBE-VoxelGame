#ifndef PLAYERBASE_HPP
#define PLAYERBASE_HPP

#include "Entity.hpp"

class PlayerBase : public Entity
{
	public:
		PlayerBase(int id);
		float yaw, pitch;
		unsigned char heldBlock;

	protected:
		int id;
};

#endif // PLAYERBASE_HPP
