#include "Frustum.hpp"

Frustum::Plane::Plane(vec3f p0, vec3f p1, vec3f p2) {
	vec3f v = p1-p0;
	vec3f u = p2-p0;
	n = glm::normalize(glm::cross(u,v));
	d = -glm::dot(n,p0);
}

Frustum::Plane::Plane(vec3f n, vec3f p) : n(glm::normalize(n)), d(-glm::dot(glm::normalize(n),p)) {
}

Frustum::Plane::Plane(vec3f n, float d) : n(glm::normalize(n)), d(d/glm::length(n)){
}

Frustum::Plane::Plane(vec4f ABCD) : n(vec3f(ABCD)), d(ABCD.z) {
	float l = glm::length(n);
	n /= l;
	d /= l;
}

Frustum::Plane::Plane() : n(0), d(0) {
}

Frustum::Plane::~Plane() {
}

bool Frustum::Plane::sphereInside(vec3f p, float r) const {
	float distance = glm::dot(n,p) + d;
	return (distance < r);
}

bool Frustum::Plane::pointInside(vec3f p) const {
	float distance = glm::dot(n,p) + d;
	return (distance < 0);
}

Frustum::Frustum() {
}

Frustum::~Frustum() {
}

bool Frustum::insideFrustum( const vec3f &center, float radius) const {
	for(uint i=0; i < 4; i++)
		if(!planes[i].sphereInside(center,radius)) return false;
	return true;
}

void Frustum::calculate(mat4f VP) {
	mat4f invVP = glm::inverse(VP);
	vec4f ntl,ntr,nbl,nbr,ftl,ftr,fbl,fbr;
	// compute the 4 corners of the frustum on the near plane
	ntl = invVP * vec4f(-1, 1,-1, 1);
	ntr = invVP * vec4f( 1, 1,-1, 1);
	nbl = invVP * vec4f(-1,-1,-1, 1);
	nbr = invVP * vec4f( 1,-1,-1, 1);

	// compute the 4 corners of the frustum on the far plane
	ftl = invVP * vec4f(-1, 1, 1, 1);
	ftr = invVP * vec4f( 1, 1, 1, 1);
	fbl = invVP * vec4f(-1,-1, 1, 1);
	fbr = invVP * vec4f( 1,-1, 1, 1);
	
	// compute the six planes
	planes[TOP]		= Plane(vec3f(ntl/ntl.w),vec3f(ftl/ftl.w),vec3f(ftr/ftr.w));
	planes[BOTTOM]	= Plane(vec3f(nbl/nbl.w),vec3f(nbr/nbr.w),vec3f(fbr/fbr.w));
	planes[LEFT]	= Plane(vec3f(nbl/nbl.w),vec3f(fbl/fbl.w),vec3f(ftl/ftl.w));
	planes[RIGHT]	= Plane(vec3f(ntr/ntr.w),vec3f(ftr/ftr.w),vec3f(fbr/fbr.w));
}
