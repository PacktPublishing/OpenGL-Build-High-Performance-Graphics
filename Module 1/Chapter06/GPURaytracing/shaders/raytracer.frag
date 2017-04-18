#version 330 core

layout(location = 0) out vec4 vFragColor; //fragment shader output

//structs for Ray, Box and Camera objects
struct Ray { vec3 origin, dir;} eyeRay; 
struct Box { vec3 min, max; };
struct Camera {
   vec3 U,V,W; 
   float d;
}cam;


//input from the vertex shader
smooth in vec2 vUV;					//interpolated texture coordinates

//shader uniforms
uniform mat4 invMVP;				//inverse of combined modelview projection matrix
uniform vec4 backgroundColor;		//background colour
uniform vec3 eyePos;				//eye position in object space
uniform sampler2D vertex_positions;	//mesh vertices
uniform isampler2D triangles_list;	//mesh triangles
uniform sampler2DArray textureMaps;	//all mesh textures
uniform vec3 light_position;		//light position is in object space
uniform Box aabb;					//scene's bounding box 
uniform float VERTEX_TEXTURE_SIZE;	//size of the vertex texture
uniform float TRIANGLE_TEXTURE_SIZE;//size of the triangle texture 
 
//shader constants
const float k0 = 1.0;	//constant attenuation
const float k1 = 0.0;	//linear attenuation
const float k2 = 0.0;	//quadratic attenuation
 
//function to return the intersection of a ray with a box
//returns a vec2 in which the x value contains the t value at the near intersection
						//the y value contains the t value at the far intersection
vec2 intersectCube(vec3 origin, vec3 ray, Box cube) {		
	vec3   tMin = (cube.min - origin) / ray;		
	vec3   tMax = (cube.max - origin) / ray;		
	vec3     t1 = min(tMin, tMax);		
	vec3     t2 = max(tMin, tMax);
	float tNear = max(max(t1.x, t1.y), t1.z);
	float  tFar = min(min(t2.x, t2.y), t2.z);
	return vec2(tNear, tFar);	
}

//gets the direction given a 2D position and a Camera
vec3 get_direction(vec2 p, Camera c) {
   return normalize(p.x * c.U + p.y * c.V + c.d * c.W);   
}

//Generates the eye ray for the given camera and a 2D position
void setup_camera(vec2 uv) {
 
  eyeRay.origin = eyePos; 
    
  cam.U = (invMVP*vec4(1,0,0,0)).xyz; 
  cam.V = (invMVP*vec4(0,1,0,0)).xyz; 
  cam.W = (invMVP*vec4(0,0,1,0)).xyz; 
  cam.d = 1;    
  
  eyeRay.dir = get_direction(uv , cam); 
  eyeRay.dir += cam.U*uv.x;
  eyeRay.dir += cam.V*uv.y;  
}

//pseudorandom number generator
float random(vec3 scale, float seed) {		
	return fract(sin(dot(gl_FragCoord.xyz + seed, scale)) * 43758.5453 + seed);	
}	

//gives a uniform random direction vector
vec3 uniformlyRandomDirection(float seed) {		
	float u = random(vec3(12.9898, 78.233, 151.7182), seed);		
	float v = random(vec3(63.7264, 10.873, 623.6736), seed);		
	float z = 1.0 - 2.0 * u;		
	float r = sqrt(1.0 - z * z);	
	float angle = 6.283185307179586 * v;	
	return vec3(r * cos(angle), r * sin(angle), z);	
}	

//returns a uniformly random vector
vec3 uniformlyRandomVector(float seed) 
{		
	return uniformlyRandomDirection(seed) * sqrt(random(vec3(36.7539, 50.3658, 306.2759), seed));	
}	

//ray triangle intesection routine. The normal is returned in the given 
//normal reference argument.
//
//The return value is a vec4 with
//x -> t value at intersection.
//y -> u texture coordinate
//z -> v texture coordinate
//w -> texture map id
vec4 intersectTriangle(vec3 origin, vec3 dir, int index,  out vec3 normal ) {
	 
	ivec4 list_pos = texture(triangles_list, vec2((index+0.5)/TRIANGLE_TEXTURE_SIZE, 0.5));
	if((index+1) % 2 !=0 ) { 
		list_pos.xyz = list_pos.zxy;
	}  
	vec3 v0 = texture(vertex_positions, vec2((list_pos.z + 0.5 )/VERTEX_TEXTURE_SIZE, 0.5)).xyz;
	vec3 v1 = texture(vertex_positions, vec2((list_pos.y + 0.5 )/VERTEX_TEXTURE_SIZE, 0.5)).xyz;
	vec3 v2 = texture(vertex_positions, vec2((list_pos.x + 0.5 )/VERTEX_TEXTURE_SIZE, 0.5)).xyz;
	  
	vec3 e1 = v1-v0;
	vec3 e2 = v2-v0;
	vec3 tvec = origin - v0;  
	
	vec3 pvec = cross(dir, e2);  
	float  det  = dot(e1, pvec);   

	float inv_det = 1.0/ det;  

	float u = dot(tvec, pvec) * inv_det;  

	if (u < 0.0 || u > 1.0)  
		return vec4(-1,0,0,0);  

	vec3 qvec = cross(tvec, e1);  

	float v = dot(dir, qvec) * inv_det;  

	if (v < 0.0 || (u + v) > 1.0)  
		return vec4(-1,0,0,0);  

	float t = dot(e2, qvec) * inv_det;
	if((index+1) % 2 ==0 ) {
		v = 1-v; 
	} else {
		u = 1-u;
	} 

	normal = normalize(cross(e2,e1));
	return vec4(t,u,v,list_pos.w);
}

//function to test if the given ray intersect any object
//if so it returns 0.5 otherwise 1. This darkens the shade
//simulating shadow
float shadow(vec3 origin, vec3 dir ) {
	vec3 tmp;
	for(int i=0;i<int(TRIANGLE_TEXTURE_SIZE);i++) 
	{
		vec4 res = intersectTriangle(origin, dir, i, tmp); 
		if(res.x>0 ) { 
		   return 0.5;   
		}
	}
	return 1.0;
}

void main()
{ 
	//set the maximum t value
	float t = 10000;  

	//set the fragment colour as the background colour 
	vFragColor = backgroundColor;

	//setup the camera for the given texture coordinate
	setup_camera(vUV);
	 
	//check if the ray intersects the scene bounding box 
	vec2 tNearFar = intersectCube(eyeRay.origin, eyeRay.dir,  aabb);

	//if we have a valid intersection
	if(tNearFar.x<tNearFar.y  ) {
		
		t = tNearFar.y+1; //offset the near intersection to remove the depth artifacts
		  
		//trace ray through the whole triangle list and see if we have a intersection
		//if there is a triangle, we do an intersection test
		 
		vec4 val=vec4(t,0,0,0);
		vec3 N;
		//brute force check all triangles
		for(int i=0;i<int(TRIANGLE_TEXTURE_SIZE);i++) 
		{
			vec3 normal;
			vec4 res = intersectTriangle(eyeRay.origin, eyeRay.dir, i, normal); 
		 	if(res.x>0 && res.x <= val.x) {
			   val = res;  
			   N = normal;
		    }
		}

		//if there is a valid intersection
		if(val.x < t) {			 
			//vFragColor = vec4(val.yz,0,1); 

			//get the hit point
			vec3 hit = eyeRay.origin + eyeRay.dir*val.x;

			//cast a random ray from the light position
			vec3  jitteredLight  =  light_position +  uniformlyRandomVector(gl_FragCoord.x);			
		
			//get the light vector
			vec3 L = (jitteredLight.xyz-hit);
			float d = length(L);
			L = normalize(L);

			//calcualte the diffuse component
			float diffuse = max(0, dot(N, L));	

			//apply attenuation
			float attenuationAmount = 1.0/(k0 + (k1*d) + (k2*d*d));
			diffuse *= attenuationAmount;

			//check for shadows
			float inShadow = shadow(hit+ N*0.0001, L) ;
			//return final color
			vFragColor = inShadow*diffuse*mix(texture(textureMaps, val.yzw), vec4(1), (val.w==255) );
        	return;
		}    
	} 
}


