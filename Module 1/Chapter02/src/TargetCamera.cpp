#define _USE_MATH_DEFINES
#include <cmath>
#include "TargetCamera.h"
#include <iostream>

#include <glm/gtx/euler_angles.hpp>

CTargetCamera::CTargetCamera(void)
{  
	right = glm::vec3(1,0,0);
	up = glm::vec3(0,1,0);
	look = glm::vec3(0,0,-1);
	minRy = -60;
	maxRy = 60;
	minDistance = 1;
	maxDistance = 10;
}


CTargetCamera::~CTargetCamera(void)
{
}
 
void CTargetCamera::Update() {
	 
	glm::mat4 R = glm::yawPitchRoll(yaw,pitch,0.0f);
	glm::vec3 T = glm::vec3(0,0,distance);
	T = glm::vec3(R*glm::vec4(T,0.0f));
	position = target + T;
	look = glm::normalize(target-position);
	up = glm::vec3(R*glm::vec4(UP,0.0f));
	right = glm::cross(look, up);
	V = glm::lookAt(position, target, up); 
}

void CTargetCamera::SetTarget(const glm::vec3 tgt) {
	target = tgt; 
	distance = glm::distance(position, target);
	distance = std::max(minDistance, std::min(distance, maxDistance));
	/*V = glm::lookAt(position, target, up);

	m_yaw = 0;
	m_pitch = 0;

	if(V[0][0] < 0)
		m_yaw = glm::degrees((float)(M_PI - asinf(-V[2][0])) );
	else
		m_yaw = glm::degrees(asinf(-V[2][0]));

	m_pitch = glm::degrees(asinf(-V[1][2]));  
	*/
}

const glm::vec3 CTargetCamera::GetTarget() const {
	return target;
} 

void CTargetCamera::Rotate(const float yaw, const float pitch, const float roll) {
 	float p = std::min( std::max(pitch, minRy), maxRy);
	CAbstractCamera::Rotate(yaw, p, roll); 
}
 
void CTargetCamera::Pan(const float dx, const float dy) {
	glm::vec3 X = right*dx;
	glm::vec3 Y = up*dy;
	position += X + Y;
	  target += X + Y;
	Update();
}

 
void CTargetCamera::Zoom(const float amount) { 
	position += look * amount;
	distance = glm::distance(position, target); 
	distance = std::max(minDistance, std::min(distance, maxDistance));
	Update();
}
 
void CTargetCamera::Move(const float dx, const float dy) {
	glm::vec3 X = right*dx;
	glm::vec3 Y = look*dy;
	position += X + Y;
	  target += X + Y;
	Update();
}