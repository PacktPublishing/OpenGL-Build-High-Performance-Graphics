#pragma once
#include "abstractcamera.h"

class CTargetCamera :
	public CAbstractCamera
{
public:
	CTargetCamera(void);
	~CTargetCamera(void);

	void Update();
	void Rotate(const float yaw, const float pitch, const float roll);
	 
	void SetTarget(const glm::vec3 tgt);
	const glm::vec3 GetTarget() const;

	void Pan(const float dx, const float dy);
	void Zoom(const float amount );
	void Move(const float dx, const float dz);

protected:
	glm::vec3 target;   
	 
	float minRy, maxRy;
	float distance;
	float minDistance, maxDistance;

};

