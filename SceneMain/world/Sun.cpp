#include "Sun.hpp"

Sun::Sun() : angle(45.0f) {
	setName("sun");
	Camera* cam = new Camera("sunCamera");
	cam->projection = glm::ortho(-10,10,-10,10,-1000,1000);
	cam->addTo(this);
}

Sun::~Sun() {
}

void Sun::update(float deltaTime) {
}
