#include "AbstractCamera.h"  
 

glm::vec3 CAbstractCamera::UP = glm::vec3(0,1,0);

CAbstractCamera::CAbstractCamera(void) 
{ 
	Znear = 0.1f;
	Zfar  = 1000;
}


CAbstractCamera::~CAbstractCamera(void)
{
}

void CAbstractCamera::SetupProjection(const float fovy, const float aspRatio, const float nr, const float fr) {
	P = glm::perspective(fovy, aspRatio, nr, fr); 
	Znear = nr;
	Zfar = fr;
	fov = fovy;
	aspect_ratio = aspRatio; 
} 

const glm::mat4 CAbstractCamera::GetViewMatrix() const {
	return V;
}

const glm::mat4 CAbstractCamera::GetProjectionMatrix() const {
	return P;
}

const glm::vec3 CAbstractCamera::GetPosition() const {
	return position;
}

void CAbstractCamera::SetPosition(const glm::vec3& p) {
	position = p;
}
  
const float CAbstractCamera::GetFOV() const {
	return fov;
} 
void CAbstractCamera::SetFOV(const float fovInDegrees) {
	fov = fovInDegrees;
	P = glm::perspective(fovInDegrees, aspect_ratio, Znear, Zfar); 
}
const float CAbstractCamera::GetAspectRatio() const {
	return aspect_ratio;
}

 void CAbstractCamera::CalcFrustumPlanes() {
 	

	glm::vec3 cN = position + look*Znear;
	glm::vec3 cF = position + look*Zfar; 

	float Hnear = 2.0f * tan(glm::radians(fov / 2.0f) ) * Znear;
	float Wnear = Hnear * aspect_ratio;
	float Hfar = 2.0f * tan(glm::radians(fov / 2.0f)) * Zfar;
	float Wfar = Hfar * aspect_ratio;
	float hHnear = Hnear/2.0f;
	float hWnear = Wnear/2.0f;
	float hHfar = Hfar/2.0f;
	float hWfar = Wfar/2.0f;


	farPts[0] = cF + up*hHfar - right*hWfar;
	farPts[1] = cF - up*hHfar - right*hWfar;
	farPts[2] = cF - up*hHfar + right*hWfar;
	farPts[3] = cF + up*hHfar + right*hWfar;

	nearPts[0] = cN + up*hHnear - right*hWnear;
	nearPts[1] = cN - up*hHnear - right*hWnear;
	nearPts[2] = cN - up*hHnear + right*hWnear;
	nearPts[3] = cN + up*hHnear + right*hWnear;

	planes[0] = CPlane::FromPoints(nearPts[3],nearPts[0],farPts[0]);
	planes[1] = CPlane::FromPoints(nearPts[1],nearPts[2],farPts[2]);
	planes[2] = CPlane::FromPoints(nearPts[0],nearPts[1],farPts[1]);
	planes[3] = CPlane::FromPoints(nearPts[2],nearPts[3],farPts[2]);
	planes[4] = CPlane::FromPoints(nearPts[0],nearPts[3],nearPts[2]);
	planes[5] = CPlane::FromPoints(farPts[3] ,farPts[0] ,farPts[1]);
 }

 bool CAbstractCamera::IsPointInFrustum(const glm::vec3& point) {
	for(int i=0; i < 6; i++) 
	{
		if (planes[i].GetDistance(point) < 0)
			return false;
	}
	return true;
}
 
 bool CAbstractCamera::IsSphereInFrustum(const glm::vec3& center, const float radius) {
	for(int i=0; i < 6; i++) 
	{
		float d = planes[i].GetDistance(center);
		if ( d < -radius)
			return false;	
	}
	return true;
 }


  bool CAbstractCamera::IsBoxInFrustum(const glm::vec3& min, const glm::vec3& max) {
	for(int i=0; i < 6; i++) 
	{
		glm::vec3 p=min, n=max;
		glm::vec3 N = planes[i].N;
		if(N.x>=0) {
			p.x = max.x;
			n.x = min.x;
		}
		if(N.y>=0) {
			p.y = max.y;
			n.y = min.y;
		}
		if(N.z>=0) {
			p.z = max.z;
			n.z = min.z;
		}

		if ( planes[i].GetDistance(p) < 0 ) {
			return false;
		} 
	}
	return true;
  }

void CAbstractCamera::GetFrustumPlanes(glm::vec4 fp[6]) {
	for(int i=0;i<6;i++) 
		fp[i]=glm::vec4(planes[i].N, planes[i].d);	
}

void CAbstractCamera::Rotate(const float y, const float p, const float r) {
	  yaw=glm::radians(y);
	pitch=glm::radians(p);
	 roll=glm::radians(r);
	Update();
}