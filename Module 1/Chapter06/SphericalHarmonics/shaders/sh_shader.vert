#version 330 core
 
layout(location = 0) in vec3 vVertex;	//object space vertex
layout(location = 1) in vec3 vNormal;	//object space normal
layout(location = 2) in vec2 vUV;		//texture coordinates
 
//shader outputs to fragment shader
smooth out vec2 vUVout;			//texture coordinates
smooth out vec4 diffuse;		//diffuse colour

//shader uniforms
uniform mat4 P;					//projection matrix
uniform mat4 MV;				//modelview matrix
uniform mat3 N;					//normal matrix
uniform vec3 light_position;	// light position in object space

//shader constants 
//C1-C2 are spherical harmonics coefficient obtained using projection of the 
//spherical harmincs basis
const float C1 = 0.429043;
const float C2 = 0.511664;
const float C3 = 0.743125;
const float C4 = 0.886227;
const float C5 = 0.247708;
const float PI = 3.1415926535897932384626433832795;


//Campus sunset HDR probe basic frequency components
const vec3 L00 = vec3(.79,.94, .98);
const vec3 L1m1 = vec3(.44, .56, .70);
const vec3 L10 = vec3(-.10, -.18, -.27);
const vec3 L11 = vec3(.45, .38, .20);
const vec3 L2m2 = vec3(.18, .14, .05);
const vec3 L2m1 = vec3(-.14, -.22, -.31);
const vec3 L20 = vec3(-.39, -.40, -.36);
const vec3 L21 = vec3(.09, .07, .04);
const vec3 L22 = vec3(.67, .67, .52);
const vec3 scaleFactor = vec3(0.39/ (0.79+0.39), 0.40/(.94+0.40), 0.31/(0.98+0.31));

//attenuation constants
const float k0 = 1.0;	//constant attenuation
const float k1 = 0.0;	//linear attenuation
const float k2 = 0.0;	//quadratic attenuation

void main()
{
	//copy the texture coordinates to output variable
	vUVout=vUV; 
	//normalize the normal vector
	vec3 tmpN = normalize(N*vNormal);  

	//get eye space light and vertex position
	vec4 vEyeSpaceLightPosition = (MV*vec4(light_position,1));
	vec4 vEyeSpacePosition = MV*vec4(vVertex,1);

	//get the light vector
	vec3 L = (vEyeSpaceLightPosition.xyz-vEyeSpacePosition.xyz);

	//get the light distance
	float d = length(L);

	//normalize the light vector
	L = normalize(L);

	//get the diffuse component from light source
	float nDotL = max(0, dot(L,tmpN));

	//estimate the diffuse component using the polynomial interpolation of the 
	//spherical harmonics coefficients with the eye space normal
	vec3 diff = C1 * L22 * (tmpN.x * tmpN.x - tmpN.y * tmpN.y) + 
				   C3 * L20 * tmpN.z * tmpN.z + 
				   C4 * L00 - 
				   C5 * L20 + 
				   2.0 * C1 * L2m2 * tmpN.x * tmpN.y + 
				   2.0 * C1 * L21 * tmpN.x * tmpN.z + 
				   2.0 * C1 * L2m1 * tmpN.y * tmpN.z + 
				   2.0 * C2 * L11 * tmpN.x +
				   2.0 * C2 * L1m1 * tmpN.y +
				   2.0 * C2 * L10 * tmpN.z;
	diff *= scaleFactor;//brings it to 0 - 1 range

	//the final diffuse component is the sum of the diffuse component from the
	//scene lighting and the diffuse component estimated using spherical harmonics
	diffuse = vec4(diff+nDotL, 1);
	
	//apply attenuation
	float attenuationAmount = 1.0/(k0 + (k1*d) + (k2*d*d));
	diffuse *= attenuationAmount;
    
	//get the clipspace position
	gl_Position = P*(vEyeSpacePosition);
}