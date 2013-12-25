#include "DeferredModel.hpp"
#include "DeferredContainer.hpp"
#include "Camera.hpp"

DeferredModel::DeferredModel(const std::string& meshID, const std::string& texID, float ambient, float specular) :
	pos(0.0f), scale(1.0f), ambient(ambient), specular(specular), lookAt(0),
	renderer((DeferredContainer*)getGame()->getObjectByName("deferred")), tex(texID) {
	model.mesh = Meshes.get(meshID);
	model.program = Programs.get("deferredModel");
}

DeferredModel::~DeferredModel() {
}

void DeferredModel::update(float deltaTime) {
	(void) deltaTime;
	transform = glm::translate(mat4f(1.0f),pos);
	if(glm::length(lookAt-pos) > 0.01 && glm::abs(glm::dot(vec3f(0,1,0),glm::normalize(lookAt-pos))) < 0.999) {
		transform = glm::translate(mat4f(1.0f),pos);
		transform *=glm::lookAt(lookAt,-pos,vec3f(0,1,0));
	}
	else
		transform = glm::translate(mat4f(1.0f),pos);
	transform = glm::scale(transform, scale);
}

void DeferredModel::draw() const {
	if(renderer->getMode() != DeferredContainer::Deferred) return;
	drawDeferred();
}

void DeferredModel::drawDeferred() const {
	Camera* cam = (Camera*)getGame()->getObjectByName("playerCam");
	model.program = Programs.get("deferredModel");
	model.program->uniform("MVP")->set(cam->projection*cam->view*fullTransform);
	model.program->uniform("M")->set(fullTransform);
	model.program->uniform("V")->set(cam->view);
	model.program->uniform("ambient")->set(ambient);
	model.program->uniform("specular")->set(specular);
	model.program->uniform("diffuseTex")->set(Textures.get(tex));
	model.draw();
}
