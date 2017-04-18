#version 330 core
precision highp float;

#extension EXT_gpu_shader4 : require

layout( location = 0 )  in vec4 position_mass; //xyz -> position, w->mass
layout( location = 1 )  in vec4 prev_position; //xyz -> prev_position, w->mass
 
uniform mat4 MVP;								//combined modelview projection matrix
uniform samplerBuffer tex_position_mass;		//buffer texture for current position
uniform samplerBuffer tex_prev_position_mass;	//buffer texture for previous position
uniform vec2  inv_cloth_size;					//size of a single patch in world space
uniform vec2  step;								//delta texture size
uniform int texsize_x;							//size of position texture
uniform int texsize_y;							

//elapsed time, spring and damping constants
uniform float dt, ksStr, ksShr, ksBnd, 
				 kdStr, kdShr, kdBnd, DEFAULT_DAMPING;
 
uniform mat4  ellipsoid_xform;		//ellipsoid's transform
uniform mat4  inv_ellipsoid;		//inverse of the ellipsoid's transform
uniform vec4  ellipsoid;			//(center in xyz, radius in w) of ellipsoid

//force due to gravity
uniform vec3 gravity;
 
//shader outputs
out vec4 out_position_mass;		
out	vec4 out_prev_position;
 
// Resolve constraint in this space
const vec3 center = vec3(0,0,0);
const float radius = 1;

//returns the position of the given vertex along with its spring and damping constants
ivec2 getNextNeighbor(int n, out float ks, out float kd) { 
   //structural springs (adjacent neighbors)
   //        o
   //        |
   //     o--m--o
   //        |
   //        o
   if(n<4) {
       ks = ksStr;
       kd = kdStr;
   }
	if (n == 0)	return ivec2( 1,  0);
	if (n == 1)	return ivec2( 0, -1);
	if (n == 2)	return ivec2(-1,  0);
	if (n == 3)	return ivec2( 0,  1);
	
	//shear springs (diagonal neighbors)
	//     o  o  o
	//      \   /
	//     o  m  o
	//      /   \
	//     o  o  o
	if(n<8) {
       ks = ksShr;
       kd = kdShr;
   }
	if (n == 4) return ivec2( 1,  -1);
	if (n == 5) return ivec2( -1, -1);	
	if (n == 6) return ivec2(-1,  1);
	if (n == 7) return ivec2( 1,  1);
	
	//bend spring (adjacent neighbors 1 node away)
	//
	//o   o   o   o   o
	//        | 
	//o   o   |   o   o
	//        |   
	//o-------m-------o
	//        |  
	//o   o   |   o   o
	//        |
	//o   o   o   o   o 
	if(n<12) {
       ks = ksBnd;
       kd = kdBnd;
   }
	if (n == 8)	return ivec2( 2, 0);
	if (n == 9) return ivec2( 0, -2);
	if (n ==10) return ivec2(-2, 0);
	if (n ==11) return ivec2( 0, 2);
}

//function for collision resolution of a sphere with a given vertex x
void sphereCollision(inout vec3 x, vec4 sphere)
{
	vec3 delta = x - sphere.xyz;
	float dist = length(delta);
	if (dist < sphere.w) {
		x = sphere.xyz + delta*(sphere.w / dist);
	}
}

//function for collision resolution of a plane with a given vertex x
void planeCollision(inout vec3 x,  vec4 plane) {
	 float dist = dot(plane.xyz,x)+ plane.w;
	 if(dist<0) {
		 x += plane.xyz*-dist;
	 }
}
 
void main() 
{  
	//get the current particle attributes
	float m = position_mass.w;
	vec3 pos = position_mass.xyz;
    vec3 pos_old = prev_position.xyz;	
	vec3 vel = (pos - pos_old) / dt;
	float ks=0, kd=0;

	//determine the index of the current particles
	int index = gl_VertexID;
	//resolve linear index into a 2D index
	int ix = index % texsize_x;
	int iy = index / texsize_x;

	//if this is a corner vertex, set its mass to 0 so it is immovable
	if(index ==0 || index == (texsize_x-1))
		m = 0;

	//calcualte external force due to gravity and velocity damping force
	//and add to net force F
    vec3 F = gravity*m + (DEFAULT_DAMPING*vel);
	
	//for all neighbors of the current vertex
	for(int k=0;k<12;k++) {
		//get neighbor coordinates
		ivec2 coord = getNextNeighbor(k, ks, kd);
		int j = coord.x;
		int i = coord.y;		
		//check for out of bounds indices
		if (((iy + i) < 0) || ((iy + i) > (texsize_y-1)))
			continue;

		if (((ix + j) < 0) || ((ix + j) > (texsize_x-1)))
			continue;

		//get linear index
		int index_neigh = (iy + i) * texsize_x + ix + j;

		//fetch the current and previous position from the buffer textures
		vec3 p2 = texelFetchBuffer(tex_position_mass, index_neigh).xyz;
		vec3 p2_last = texelFetchBuffer(tex_prev_position_mass, index_neigh).xyz;
		
		//get the absolute coordinates
		vec2 coord_neigh = vec2(ix + j, iy + i)*step;
		
		//determine the restlength
		float rest_length = length(coord*inv_cloth_size);

		//calculate the velocity and change in position and velocity
		vec3 v2 = (p2- p2_last)/dt;
		vec3 deltaP = pos - p2;	
		vec3 deltaV = vel - v2;	 
		float dist = length(deltaP);
		
		//use the rest length, change in position and velocity to get the internal (spring) force
		float   leftTerm = -ks * (dist-rest_length);
		float  rightTerm = kd * (dot(deltaV, deltaP)/dist);		
		vec3 springForce = (leftTerm + rightTerm)* normalize(deltaP);

		//add internal force to the net force
		F +=  springForce;	
	}
	//calculate the acceleration
    vec3 acc = vec3(0);
	if(m!=0)
	   acc = F/m; 

	//calculate new position using Verlet method
	vec3 tmp = pos; 
	pos = pos * 2.0 - pos_old + acc* dt * dt;
	pos_old = tmp;

	//collision with floor replaced with the plane collision
	//pos.y=max(0, pos.y);

	//collision with plane
	//planeCollision(pos, vec4(0,1,0,0));

	//collision with sphere
	//sphereCollision(pos, vec4(0,2,0,1)) ;

	//collision with box
	///boxCollision(pos, pos_old, vec3(0,0,0), vec3(1,1,1));

	
	//apply collision to the ellipsoid	
	//bring the current position to the ellipsoid object space
	vec4 x0 = inv_ellipsoid*vec4(pos,1); 

	//calcualte the vector from the ellipsoid center
	vec3 delta0 = x0.xyz-ellipsoid.xyz;
	float dist2 = dot(delta0, delta0);
	//if the distance is < 1, we have a collision
	if(dist2<1) {  
		//determine the penetration amount 
		delta0 = (ellipsoid.w - dist2) * delta0 / dist2;

		//transform the delta back to original space
		vec3 delta;
		vec3 transformInv = vec3(ellipsoid_xform[0].x, ellipsoid_xform[1].x, ellipsoid_xform[2].x);
		transformInv /= dot(transformInv, transformInv);

		delta.x = dot(delta0, transformInv);
		transformInv = vec3(ellipsoid_xform[0].y, ellipsoid_xform[1].y, ellipsoid_xform[2].y);
		transformInv /= dot(transformInv, transformInv);

		delta.y = dot(delta0, transformInv);
		transformInv = vec3(ellipsoid_xform[0].z, ellipsoid_xform[1].z, ellipsoid_xform[2].z);
		transformInv /= dot(transformInv, transformInv);

		delta.z = dot(delta0, transformInv); 
		//move the current position with delta amount
		pos +=  delta ; 

		//set the previous position as current postiion so that in effect the net velocity is 0
		pos_old = pos; //this is added so that the net velocity is zero in the next iteration
					   //removing this will cause points to continuously popup at the collision
	}
	
	//set the shader outputs and clip space position
	out_position_mass = vec4(pos, m);	
	out_prev_position = vec4(pos_old,m);			  
	gl_Position = MVP*vec4(pos, 1);
	

	/*
	//Debug
	out_position_mass = vec4(ix,iy,0,1);
	out_prev_position = out_position_mass ;
	gl_Position = MVP*vec4(pos,1.0);
	*/
}