#include "Frustum.hpp"

Frustum::Plane::Plane(vec3f p0, vec3f p1, vec3f p2) {
	vec3f v = p1-p0;
	vec3f u = p2-p0;
	n = glm::normalize(glm::cross(u,v));
	d = -glm::dot(n,p0);
}

Frustum::Plane::Plane(vec3f n, vec3f p) : n(n), d(-glm::dot(n,p)) {
}

Frustum::Plane::Plane(vec3f n, float d) : n(n), d(d){
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

void Frustum::calculate(mat4f viewMatrix) {
	//calculate frustum with dir, pos , znear, zfar, fov, screen ratio
	vec3f
			dir(viewMatrix[0][2], viewMatrix[1][2], viewMatrix[2][2]),//same as the player's pov
			side(viewMatrix[0][0], viewMatrix[1][0], viewMatrix[2][0]), //x of the camera in world coords
			up(viewMatrix[0][1], viewMatrix[1][1], viewMatrix[2][1]); //up vector of the camera in world coords

	float tfov = (float)tan(DEG_TO_RAD * FOV * 0.5) ; //half FOV in rads
	float ratio = float(SCRWIDTH)/float(SCRHEIGHT);
	vec3f camPos = vec3f(glm::inverse(viewMatrix)*vec4f(0,0,0,1));

	float nh,nw,fh,fw;
	vec3f
			nc,fc,
			ntl,ntr,nbl,nbr,
			ftl,ftr,fbl,fbr;
	nh = ZNEAR * tfov; //near height
	nw = nh * ratio;   //near width
	fh = ZFAR  * tfov; //far height
	fw = fh * ratio;   //far width

	// compute the centers of the near and far planes
	nc = camPos - dir * ZNEAR;
	fc = camPos - dir * ZFAR;

	// compute the 4 corners of the frustum on the near plane
	ntl = nc + (up * nh) - (side * nw);
	ntr = nc + (up * nh) + (side * nw);
	nbl = nc - (up * nh) - (side * nw);
	nbr = nc - (up * nh) + (side * nw);

	// compute the 4 corners of the frustum on the far plane
	ftl = fc + (up * fh) - (side * fw);
	ftr = fc + (up * fh) + (side * fw);
	fbl = fc - (up * fh) - (side * fw);
	fbr = fc - (up * fh) + (side * fw);

	// compute the six planes
	planes[TOP] = Plane(ntl,ftl,ftr);
	planes[BOTTOM] = Plane(nbl,nbr,fbr);
	planes[LEFT] = Plane(nbl,fbl,ftl);
	planes[RIGHT] = Plane(ntr,ftr,fbr);
}