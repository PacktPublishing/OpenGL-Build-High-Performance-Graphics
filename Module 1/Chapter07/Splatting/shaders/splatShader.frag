#version 330 core

layout(location = 0) out vec4 vFragColor;	//fragment shader output

smooth in vec3 outNormal;	//eye space per-vertex normal 
							//interpolated varying input from vertex shader
  
//constants
const vec3 L = vec3(0,0,1);	//light vector
const vec3 V = L;			//view vector
const vec4 diffuse_color = vec4(0.75,0.5,0.5,1);	//surface diffuse colour
const vec4 specular_color = vec4(1);				//surface specular colour

void main()
{ 
	//get the normalized eye space normal
	vec3 N;
	N = normalize(outNormal);
	//get the point coordinate and check if the current fragment is outside
	//the spehre
	vec2 P = gl_PointCoord*2.0 - vec2(1.0);  
	float mag = dot(P.xy,P.xy); 

	//if so, discard the fragment. This renders the splat as circular point
	if (mag > 1) 
		discard;   

	//calculate the diffuse and specular components
	float diffuse = max(0, dot(N,L));	
    vec3 halfVec = normalize(L+V);
	float specular = pow(max(0, dot(halfVec,N)),400);	

	//set the final fragment shader by combining the diffuse and specular components
	vFragColor =  (specular*specular_color) + (diffuse*diffuse_color);
}	