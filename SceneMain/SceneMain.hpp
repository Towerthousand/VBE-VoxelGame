#ifndef SCENEMAIN_HPP
#define SCENEMAIN_HPP
#include "commons.hpp"

class SceneMain : public GameObject {
	public:
		SceneMain();
		~SceneMain();
		void update(float deltaTime);

	private:
        void loadResources();
		float debugCounter;
		int fpsCount;
};

#endif // SCENEMAIN_HPP
