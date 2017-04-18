#pragma once
#include <glm/glm.hpp>

class CPlane
{
	enum Where {COPLANAR, FRONT, BACK};

public:
	CPlane(void);
	CPlane(const glm::vec3& N, const glm::vec3& p);
	~CPlane(void);

	static CPlane FromPoints(const glm::vec3& v1, const glm::vec3& v2, const glm::vec3& v3);
	Where Classify(const glm::vec3& p);
	float GetDistance(const glm::vec3& p);


	glm::vec3 N;
	float d;
};

