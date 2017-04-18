#version 330 core
  
layout(location = 0) in vec3 vVertex;	//object space vertex position
layout(location = 1) in vec3 vNormal;	//object space vertex normal
   
//uniforms
uniform mat4 MV;			//modelview matrix
uniform mat3 N;				//normal matrix
uniform mat4 P;				//projection matrix		
uniform float splatSize;	//splat size

smooth out vec3 outNormal;	//output eye space normal

void main()
{    
	//get eye space vertex position
	vec4 eyeSpaceVertex = MV*vec4(vVertex,1);
	
	//get the splat size by using the eye space vertex z component
	gl_PointSize = 2*splatSize/-eyeSpaceVertex.z; 
	
	//get the clipspace position
	gl_Position = P * eyeSpaceVertex; 

	//get the eye space normal
    outNormal = N*vNormal;
}