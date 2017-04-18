#include "Plane.h"

const float EPSILON= 0.0001f;

CPlane::CPlane(void)
{
	N = glm::vec3(0,1,0);
	d = 0;
}


CPlane::~CPlane(void)
{
}

CPlane::CPlane(const glm::vec3& normal, const glm::vec3& p) {
	N = normal;
	d = -glm::dot(N,p);
}

CPlane CPlane::FromPoints(const glm::vec3& v1, const glm::vec3& v2, const glm::vec3& v3) {
	CPlane temp;
	glm::vec3 e1 = v2-v1;
	glm::vec3 e2 = v3-v1;
	temp.N = glm::normalize(glm::cross(e1,e2)); 
	temp.d = -glm::dot(temp.N, v1);
	return temp;
}

float CPlane::GetDistance(const glm::vec3& p) {
	return glm::dot(N,p)+d;
}

CPlane::Where CPlane::Classify(const glm::vec3& p) {
	float res = GetDistance(p);
	if( res > EPSILON)
		return FRONT;	
	else if(res < EPSILON)
		return BACK;
	else
		return COPLANAR;
}
