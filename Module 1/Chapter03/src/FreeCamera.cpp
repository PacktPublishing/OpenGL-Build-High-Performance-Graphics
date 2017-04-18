#include "FreeCamera.h"
#include <glm/gtx/euler_angles.hpp>

CFreeCamera::CFreeCamera()
{
	translation =glm::vec3(0); 
	speed = 0.5f; // 0.5 m/s
}


CFreeCamera::~CFreeCamera(void)
{
}
 
void CFreeCamera::Update() {
	glm::mat4 R = glm::yawPitchRoll(yaw,pitch,roll); 
	position+=translation;

	//set this when no movement decay is needed
	//translation=glm::vec3(0); 

	look = glm::vec3(R*glm::vec4(0,0,1,0));	
	up   = glm::vec3(R*glm::vec4(0,1,0,0));
	right = glm::cross(look, up);

	glm::vec3 tgt  = position+look;
	V = glm::lookAt(position, tgt, up); 
}




void CFreeCamera::Walk(const float dt) {
	translation += (look*speed*dt);
	Update();
}

void CFreeCamera::Strafe(const float dt) {
	translation += (right*speed*dt);
	Update();
}

void CFreeCamera::Lift(const float dt) {
	translation += (up*speed*dt);
	Update();
}
 
void CFreeCamera::SetTranslation(const glm::vec3& t) {
	translation = t;
	Update();
}

glm::vec3 CFreeCamera::GetTranslation() const {
	return translation;
}

void CFreeCamera::SetSpeed(const float s) {
	speed = s;
}

const float CFreeCamera::GetSpeed() const {
	return speed;
}